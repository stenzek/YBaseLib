#pragma once
#include "YBaseLib/Sockets/Common.h"
#include "YBaseLib/Sockets/BaseSocket.h"
#include "YBaseLib/Sockets/SocketAddress.h"
#include "YBaseLib/Error.h"

#ifdef Y_SOCKET_IMPLEMENTATION_GENERIC

class ListenSocket;

class StreamSocket : public BaseSocket
{
public:
    StreamSocket();
    virtual ~StreamSocket();

    virtual void Close() override final;

    size_t Read(void *pBuffer, size_t bufferSize);
    size_t Write(const void *pBuffer, size_t bufferSize);

protected:
    virtual void OnConnected();
    virtual void OnDisconnected(Error *pError);
    virtual void OnRead();

private:
    virtual void OnReadEvent() override final;
    virtual void OnWriteEvent() override final;

    bool InitializeSocket(SocketMultiplexer *pMultiplexer, int fileDescriptor, Error *pError);
    void CloseWithError();

protected:
    SocketMultiplexer *m_pMultiplexer;
    SocketAddress m_localAddress;
    SocketAddress m_remoteAddress;
    int m_fileDescriptor;
    bool m_connected;

    // Ugly, but needed in order to call the events.
    friend SocketMultiplexer;
    friend ListenSocket;
};

#endif      // Y_SOCKET_IMPLEMENTATION_GENERIC
