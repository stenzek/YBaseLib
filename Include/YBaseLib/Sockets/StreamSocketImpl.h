#pragma once
#include "YBaseLib/Common.h"

class Error;
struct SocketAddress;
class StreamSocket;

class StreamSocketImpl
{
public:
    // Virtual destructor
    ~StreamSocketImpl() {}

    // Connection endpoint
    virtual const SocketAddress *GetLocalAddress() = 0;
    virtual const SocketAddress *GetRemoteAddress() = 0;

    // Send/Receive
    virtual bool Read(void *pBuffer, size_t *pByteCount, Error *pError) = 0;
    virtual bool Write(const void *pBuffer, size_t *pByteCount, Error *pError) = 0;

    // Connection state
    virtual void Close() = 0;
};

