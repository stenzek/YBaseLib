#include "YBaseLib/Sockets/BufferedStreamSocket.h"
#include "YBaseLib/Sockets/SocketMultiplexer.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
Log_SetChannel(BufferedStreamSocket);

#ifdef Y_SOCKET_IMPLEMENTATION_GENERIC

// Windows-isms
#ifndef Y_PLATFORM_WINDOWS
    #define ioctlsocket ioctl
    #define closesocket close
    #define WSAEWOULDBLOCK EAGAIN
    #define WSAGetLastError() errno
#endif

BufferedStreamSocket::BufferedStreamSocket(size_t receiveBufferSize /*= 16384*/, size_t sendBufferSize /*= 16384*/)
    : m_receiveBuffer(receiveBufferSize)
    , m_sendBuffer(sendBufferSize)
{

}

BufferedStreamSocket::~BufferedStreamSocket()
{

}

size_t BufferedStreamSocket::Read(void *pBuffer, size_t bufferSize)
{
    m_lock.Lock();
    
    // Read from receive buffer.
    size_t bytesToRead = Min(m_receiveBuffer.GetContiguousUsedBytes(), bufferSize);
    if (bytesToRead > 0)
    {
        if (!m_receiveBuffer.Read(pBuffer, bytesToRead))
            bytesToRead = 0;
    }

    m_lock.Unlock();
    return bytesToRead;
}

size_t BufferedStreamSocket::Write(const void *pBuffer, size_t bufferSize)
{
    m_lock.Lock();

    // Write buffer currently being used?
    // Don't send out-of-order, push to the write buffer.
    size_t writtenBytes = 0;
    if (m_sendBuffer.GetBufferUsed() == 0)
    {
        // Send as many bytes as possible immediately. If this fails, push to the write buffer.
        int res = send(m_fileDescriptor, (const char *)pBuffer, bufferSize, 0);
        writtenBytes = (size_t)Max(res, 0);

        // Socket error?
        if (res < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            CloseWithError();
            m_lock.Unlock();
            return 0;
        }

    }

    // Any bytes left over?
    if (writtenBytes < bufferSize)
    {
        // Copy remaining bytes into write buffer, and register for write notifications.
        size_t bytesRemaining = bufferSize - writtenBytes;
        size_t bufferSpace = m_sendBuffer.GetContiguousBufferSpace();
        bool registerForWrites = (m_sendBuffer.GetBufferUsed() == 0);

        // Write to buffer.
        size_t bytesToWriteToBuffer = Min(bufferSpace, bytesRemaining);
        if (m_sendBuffer.Write((const byte *)pBuffer + writtenBytes, bytesToWriteToBuffer))
            writtenBytes += bytesToWriteToBuffer;

        // Register for write notifications.
        if (registerForWrites)
            m_pMultiplexer->SetNotificationMask(this, m_fileDescriptor, SocketMultiplexer::EventType_Read | SocketMultiplexer::EventType_Write);
    }

    m_lock.Unlock();
    return writtenBytes;
}

bool BufferedStreamSocket::AcquireReadBuffer(const void **ppBuffer, size_t *pBytesAvailable)
{
    m_lock.Lock();
    if (!m_receiveBuffer.GetReadPointer(ppBuffer, pBytesAvailable))
    {
        m_lock.Unlock();
        return false;
    }

    return true;
}

void BufferedStreamSocket::ReleaseReadBuffer(size_t bytesConsumed)
{
    DebugAssert(bytesConsumed <= m_receiveBuffer.GetContiguousBufferSpace());
    if (bytesConsumed >= 0)
        m_receiveBuffer.MoveReadPointer(bytesConsumed);

    m_lock.Unlock();
}

void BufferedStreamSocket::OnConnected()
{
    StreamSocket::OnConnected();
}

void BufferedStreamSocket::OnDisconnected(Error *pError)
{
    StreamSocket::OnDisconnected(pError);
}

void BufferedStreamSocket::OnReadEvent()
{
    m_lock.Lock();

    if (m_connected)
        OnRead();

    m_lock.Unlock();
}

void BufferedStreamSocket::OnWriteEvent()
{
    m_lock.Lock();

    // Try to send as many bytes as possible from the write buffer.
    while (m_sendBuffer.GetBufferUsed() > 0)
    {
        size_t contiguousBytes;
        const void *pBuffer;
        if (m_sendBuffer.GetReadPointer(&pBuffer, &contiguousBytes))
        {
            int res = send(m_fileDescriptor, (const char *)pBuffer, contiguousBytes, 0);
            if (res < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
            {
                CloseWithError();
                m_lock.Unlock();
                return;
            }

            // Any bytes written?
            if (res > 0)
            {
                m_sendBuffer.MoveWritePointer((size_t)res);

                // Try again only if we did a whole write.
                if ((size_t)res == contiguousBytes)
                    continue;
            }
        }

        // Don't try again.
        break;
    }

    // Do we still have bytes? If not, unregister for read notifications.
    if (m_sendBuffer.GetBufferUsed() == 0)
        m_pMultiplexer->SetNotificationMask(this, m_fileDescriptor, SocketMultiplexer::EventType_Read);
    
    m_lock.Unlock();
}

#endif      // #ifdef Y_SOCKET_IMPLEMENTATION_GENERIC
