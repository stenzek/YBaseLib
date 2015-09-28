#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/ReferenceCounted.h"

class SocketAddress;
class StreamSocketTransport;
class SocketMultiplexer;

class StreamSocket : public ReferenceCounted
{
public:
    StreamSocket(StreamSocketTransport *pTransport = nullptr);
    virtual ~StreamSocket();

    bool Connect(const SocketAddress *pAddress);
    bool BeginConnect(const SocketAddress *pAddress);

    uint32 Send(const void *pBuffer, size_t bufferLength);

protected:
    virtual void OnConnected();
    virtual void OnDisconnected();
    virtual void OnRead();

protected:
    StreamSocketTransport *m_pTransport;
    bool m_connected;
};
