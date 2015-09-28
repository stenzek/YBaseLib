#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/ReferenceCounted.h"

struct SocketAddress;
class StreamSocketImpl;
class SocketMultiplexer;

class StreamSocket : public ReferenceCounted
{
public:
    StreamSocket(SocketMultiplexer *pMultiplexer, StreamSocketImpl *pImpl);
    virtual ~StreamSocket();

    size_t Read(void *pBuffer, size_t bufferSize);
    size_t Write(const void *pBuffer, size_t bufferSize);
    void Close();

protected:
    virtual void OnConnected();
    virtual void OnDisconnected();
    virtual void OnRead();

protected:
    SocketMultiplexer *m_pMultiplexer;
    StreamSocketImpl *m_pImpl;
    bool m_connected;

    // Ugly, but needed in order to call the events.
    friend SocketMultiplexer;
};
