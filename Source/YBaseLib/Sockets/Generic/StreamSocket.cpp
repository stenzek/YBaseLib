#include "YBaseLib/Sockets/StreamSocket.h"
#include "YBaseLib/Sockets/SocketMultiplexer.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
Log_SetChannel(StreamSocket);

#ifdef Y_SOCKET_IMPLEMENTATION_GENERIC

// Windows-isms
#ifndef Y_PLATFORM_WINDOWS
    #define ioctlsocket ioctl
    #define closesocket close
    #define WSAEWOULDBLOCK EAGAIN
    #define WSAGetLastError() errno
#endif

StreamSocket::StreamSocket()
    : BaseSocket()
    , m_pMultiplexer(nullptr)
    , m_fileDescriptor(-1)
    , m_connected(false)
{

}

StreamSocket::~StreamSocket()
{
    DebugAssert(m_fileDescriptor < 0);
}

size_t StreamSocket::Read(void *pBuffer, size_t bufferSize)
{
    m_lock.Lock();
    if (!m_connected)
        return 0;

    // try a read
    int len = recv(m_fileDescriptor, (char *)pBuffer, bufferSize, 0);
    if (len <= 0)
    {
        // Check for EAGAIN
        if (len < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
        {
            // Not an error. Just means no data is available.
            m_lock.Unlock();
            return 0;
        }

        // error
        CloseWithError();
        m_lock.Unlock();
        return 0;
    }

    m_lock.Unlock();
    return len;
}

size_t StreamSocket::Write(const void *pBuffer, size_t bufferLength)
{
    m_lock.Lock();

    if (!m_connected)
    {
        m_lock.Unlock();
        return 0;
    }

    // try a write
    int len = send(m_fileDescriptor, (const char *)pBuffer, bufferLength, 0);
    if (len <= 0)
    {
        // Check for EAGAIN
        if (len < 0 && WSAGetLastError() == WSAEWOULDBLOCK)
        {
            // Not an error. Just means no data is available.
            m_lock.Unlock();
            return 0;
        }

        // error
        CloseWithError();
        m_lock.Unlock();
        return 0;
    }

    m_lock.Unlock();
    return len;
}

void StreamSocket::Close()
{
    m_lock.Lock();

    if (!m_connected)
    {
        m_lock.Unlock();
        return;
    }

    m_pMultiplexer->SetNotificationMask(this, m_fileDescriptor, 0);
    m_pMultiplexer->RemoveOpenSocket(this);
    closesocket(m_fileDescriptor);
    m_fileDescriptor = -1;
    m_connected = false;

    Error error;
    error.SetErrorNone();
    OnDisconnected(&error);

    m_lock.Unlock();
}

void StreamSocket::CloseWithError()
{
    m_lock.Lock();

    DebugAssert(m_connected);

    Error error;
    int errorCode = WSAGetLastError();
    if (errorCode == 0)
        error.SetErrorUser((int32)0, "Connection closed by peer.");
    else
        error.SetErrorSocket(errorCode);

    m_pMultiplexer->SetNotificationMask(this, m_fileDescriptor, 0);
    m_pMultiplexer->RemoveOpenSocket(this);
    closesocket(m_fileDescriptor);
    m_fileDescriptor = -1;
    m_connected = false;

    OnDisconnected(&error);

    m_lock.Unlock();
}

void StreamSocket::OnConnected()
{
    m_connected = true;
}

void StreamSocket::OnDisconnected(Error *pError)
{
    m_connected = false;
}

void StreamSocket::OnRead()
{

}

void StreamSocket::OnReadEvent()
{
    // forward through
    m_lock.Lock();
    
    if (m_connected)
        OnRead();

    m_lock.Unlock();
}

void StreamSocket::OnWriteEvent()
{
    // shouldn't be called
}

bool StreamSocket::InitializeSocket(SocketMultiplexer *pMultiplexer, int fileDescriptor, Error *pError)
{
    DebugAssert(m_pMultiplexer == nullptr);
    m_pMultiplexer = pMultiplexer;
    m_fileDescriptor = fileDescriptor;
    m_connected = true;

    // get local address
    sockaddr_storage sa;
    socklen_t salen = sizeof(sa);
    if (getsockname(m_fileDescriptor, (sockaddr *)&sa, &salen) == 0)
        m_localAddress.SetFromSockaddr(&sa, salen);
    else
        m_localAddress.SetUnknown();

    // get remote address
    salen = sizeof(sockaddr_storage);
    if (getpeername(m_fileDescriptor, (sockaddr *)&sa, &salen) == 0)
        m_remoteAddress.SetFromSockaddr(&sa, salen);
    else
        m_remoteAddress.SetUnknown();

    // switch to nonblocking mode
    unsigned long value = 1;
    if (ioctlsocket(m_fileDescriptor, FIONBIO, &value) < 0)
    {
        if (pError != nullptr)
            pError->SetErrorSocket(WSAGetLastError());

        return false;
    }

    // register for notifications
    m_pMultiplexer->AddOpenSocket(this);
    m_pMultiplexer->SetNotificationMask(this, m_fileDescriptor, SocketMultiplexer::EventType_Read);

    // trigger connected notitifcation
    m_lock.Lock();
    OnConnected();
    m_lock.Unlock();
    return true;
}


#endif      // Y_SOCKET_IMPLEMENTATION_GENERIC
