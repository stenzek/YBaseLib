#include "YBaseLib/Sockets/GenericSocketMultiplexer.h"
#include "YBaseLib/Sockets/SystemHeaders.h"
#include "YBaseLib/NumericLimits.h"
#include "YBaseLib/Log.h"
Log_SetChannel(GenericSocketMultiplexer);

// Windows-isms
#ifndef Y_PLATFORM_WINDOWS
    #define ioctlsocket ioctl
    #define closesocket close
    #define WSAEAGAIN EAGAIN
    #define WSAGetLastError() errno
#endif

GenericSocketMultiplexer::GenericListenSocket::GenericListenSocket(GenericSocketMultiplexer *pMultiplexer, CreateStreamSocketCallback acceptCallback, const SocketAddress *pAddress, int fileDescriptor)
    : m_pMultiplexer(pMultiplexer)
    , m_acceptCallback(acceptCallback)
    , m_localAddress(*pAddress)
    , m_fileDescriptor(fileDescriptor)
{
    
}

GenericSocketMultiplexer::GenericListenSocket::~GenericListenSocket()
{
    // Should be closed at destruction time
    DebugAssert(m_fileDescriptor < 0);
}

void GenericSocketMultiplexer::GenericListenSocket::Close()
{
    if (m_fileDescriptor < 0)
        return;

    closesocket(m_fileDescriptor);
    m_fileDescriptor = -1;

    // remove from list
    GenericSocketMultiplexer *pMultiplexer = m_pMultiplexer;
    pMultiplexer->m_boundSocketLock.Lock();
    for (uint32 i = 0; i < m_pMultiplexer->m_boundListenSockets.GetSize(); i++)
    {
        BoundListenSocket *pBind = &pMultiplexer->m_boundListenSockets[i];
        if (pBind->FileDescriptor == m_fileDescriptor)
        {
            //m_pMultiplexer->m_boundListenSockets[i].pSocket->

            // this release could actually delete us, so do it last.
            pMultiplexer->m_boundListenSockets.OrderedRemove(i);
            pMultiplexer->m_boundSocketLock.Unlock();
            pBind->pSocket->Release();
            return;
        }
    }

    // if we're here, we weren't in the list :S
    pMultiplexer->m_boundSocketLock.Unlock();
    Panic("Internal listen socket list corruption.");
}

void GenericSocketMultiplexer::GenericListenSocket::OnRead()
{
    // accept a connection blah
}

GenericSocketMultiplexer::GenericStreamSocketImpl::GenericStreamSocketImpl(GenericSocketMultiplexer *pMultiplexer, const SocketAddress *pLocalAddress, const SocketAddress *pRemoteAddress, int fileDescriptor)
    : m_pMultiplexer(pMultiplexer)
    , m_localAddress(*pLocalAddress)
    , m_remoteAddress(*pRemoteAddress)
    , m_fileDescriptor(fileDescriptor)
{

}

GenericSocketMultiplexer::GenericStreamSocketImpl::~GenericStreamSocketImpl()
{
    // Should be closed at destruction time
    DebugAssert(m_fileDescriptor < 0);
}

bool GenericSocketMultiplexer::GenericStreamSocketImpl::Read(void *pBuffer, size_t *pByteCount, Error *pError)
{
    // 
    return false;
}

bool GenericSocketMultiplexer::GenericStreamSocketImpl::Write(const void *pBuffer, size_t *pByteCount, Error *pError)
{
    return false;
}

void GenericSocketMultiplexer::GenericStreamSocketImpl::Close()
{
    if (m_fileDescriptor < 0)
        return;

    closesocket(m_fileDescriptor);
    m_fileDescriptor = -1;

    // remove from list
    GenericSocketMultiplexer *pMultiplexer = m_pMultiplexer;
    pMultiplexer->m_boundSocketLock.Lock();
    for (uint32 i = 0; i < m_pMultiplexer->m_boundStreamSockets.GetSize(); i++)
    {
        BoundStreamSocket *pBind = &pMultiplexer->m_boundStreamSockets[i];
        if (pBind->FileDescriptor == m_fileDescriptor)
        {
            // this release could actually delete us, so do it last.
            pMultiplexer->m_boundStreamSockets.OrderedRemove(i);
            pMultiplexer->m_boundSocketLock.Unlock();
            pBind->pSocket->Release();
            return;
        }
    }

    // if we're here, we weren't in the list :S
    pMultiplexer->m_boundSocketLock.Unlock();
    Panic("Internal stream socket list corruption.");
}

GenericSocketMultiplexer::GenericSocketMultiplexer()
{

}

GenericSocketMultiplexer::~GenericSocketMultiplexer()
{
    // close all sockets
    while (m_boundListenSockets.GetSize() > 0)
    {
        const BoundListenSocket *pBind = &m_boundListenSockets.LastElement();
        pBind->pSocket->Close();
    }
    while (m_boundStreamSockets.GetSize() > 0)
    {
        const BoundStreamSocket *pBind = &m_boundStreamSockets.LastElement();
        pBind->pSocket->Close();
    }
}

ListenSocket *GenericSocketMultiplexer::InternalCreateListenSocket(const SocketAddress *pAddress, CreateStreamSocketCallback acceptCallback)
{
    return nullptr;
}

StreamSocketImpl *GenericSocketMultiplexer::InternalConnectStreamSocket(const SocketAddress *pConnect, CreateStreamSocketCallback acceptCallback)
{
    return nullptr;
}

StreamSocketImpl *GenericSocketMultiplexer::InternalBeginConnectStreamSocket(const SocketAddress *pConnect, CreateStreamSocketCallback acceptCallback)
{
    return nullptr;
}

void GenericSocketMultiplexer::PollEvents(uint32 milliseconds)
{
    fd_set fds;
    FD_ZERO(&fds);

    // fill stuff
    int maxfd = -1;
    m_boundSocketLock.Lock();
    for (BoundListenSocket &bind : m_boundListenSockets)
    {
        FD_SET(bind.FileDescriptor, &fds);
        maxfd = Max(bind.FileDescriptor, maxfd);
    }
    for (BoundStreamSocket &bind : m_boundStreamSockets)
    {
        FD_SET(bind.FileDescriptor, &fds);
        maxfd = Max(bind.FileDescriptor, maxfd);
    }
    m_boundSocketLock.Unlock();

    // call select
    timeval tv;
    tv.tv_sec = milliseconds / 1000;
    tv.tv_usec = (milliseconds % 1000) * 1000;
    int result = select(maxfd + 1, &fds, nullptr, nullptr, (milliseconds != Y_UINT32_MAX) ? &tv : nullptr);
    if (result <= 0)
        return;

    // find sockets that triggered
    ListenSocket *pListenSockets = (ListenSocket *)alloca(sizeof(ListenSocket *) * result);
    m_boundSocketLock.
}

