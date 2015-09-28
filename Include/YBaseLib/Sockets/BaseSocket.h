#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/ReferenceCounted.h"

struct SocketAddress;
class StreamSocketImpl;
class SocketMultiplexer;

class BaseSocket : public ReferenceCounted
{
public:
    BaseSocket(SocketMultiplexer *pMultiplexer);
    virtual ~BaseSocket();

protected:
    virtual void OnReadEvent() = 0;
    virtual void OnWriteEvent() = 0;

protected:
    SocketMultiplexer *m_pMultiplexer;

    // Ugly, but needed in order to call the events.
    friend SocketMultiplexer;
};
