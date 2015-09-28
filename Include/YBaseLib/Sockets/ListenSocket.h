#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/ReferenceCounted.h"

struct SocketAddress;
class SocketMultiplexer;
class StreamSocketImpl;

class ListenSocket : public ReferenceCounted
{
public:
    virtual const SocketAddress *GetLocalAddress() const = 0;
    virtual uint32 GetConnectionsAccepted() const = 0;
    virtual void Close() = 0;

protected:
    virtual void OnRead() = 0;
    friend SocketMultiplexer;
};
