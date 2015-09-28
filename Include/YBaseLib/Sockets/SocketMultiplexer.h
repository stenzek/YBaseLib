#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/Sockets/SocketAddress.h"
#include "YBaseLib/Sockets/BaseSocket.h"
#include "YBaseLib/Sockets/ListenSocket.h"
#include "YBaseLib/Sockets/StreamSocket.h"
#include "YBaseLib/Sockets/StreamSocketImpl.h"

enum SOCKET_MULTIPLEXER_TYPE
{
    SOCKET_MULTIPLEXER_TYPE_GENERIC,
    //SOCKET_MULTIPLEXER_TYPE_EPOLL,
    //SOCKET_MULTIPLEXER_TYPE_KQUEUE,
    //SOCKET_MULTIPLEXER_TYPE_IOCP,
    NUM_SOCKET_MULTIPLEXER_TYPES
};

class SocketMultiplexer : public ReferenceCounted
{
public:
    typedef StreamSocket *(*CreateStreamSocketCallback)(const SocketAddress *pAddress, StreamSocketImpl *pImpl);

public:
    SocketMultiplexer() {}
    virtual ~SocketMultiplexer() {}

    // Access the type of multiplexer
    virtual SOCKET_MULTIPLEXER_TYPE GetType() const = 0;

    // Factory method.
    static SocketMultiplexer *Create(SOCKET_MULTIPLEXER_TYPE type = NUM_SOCKET_MULTIPLEXER_TYPES);

    // Public interface
    template<class T> ListenSocket *CreateListenSocket(const SocketAddress *pAddress);
    template<class T> T *ConnectStreamSocket(const SocketAddress *pAddress);
    template<class T> T *BeginConnectStreamSocket(const SocketAddress *pAddress);

    // Poll for events, set to Y_UINT32_MAX for infinite pause
    virtual void PollEvents(uint32 milliseconds) = 0;

protected:
    // Internal interface
    virtual ListenSocket *InternalCreateListenSocket(const SocketAddress *pAddress, CreateStreamSocketCallback acceptCallback) = 0;
    virtual StreamSocketImpl *InternalConnectStreamSocket(const SocketAddress *pConnect, CreateStreamSocketCallback acceptCallback) = 0;
    virtual StreamSocketImpl *InternalBeginConnectStreamSocket(const SocketAddress *pConnect, CreateStreamSocketCallback acceptCallback) = 0;

protected:
    // Needed because the events are protected.
    static inline void CallOnReadEvent(BaseSocket *pSocket) { pSocket->OnReadEvent(); }
    static inline void CallOnWriteEvent(BaseSocket *pSocket) { pSocket->OnWriteEvent(); }
    static inline void CallOnRead(ListenSocket *pSocket) { pSocket->OnRead(); }
    static inline void CallOnConnected(StreamSocket *pSocket) { pSocket->OnConnected(); }
    static inline void CallOnDisconnected(StreamSocket *pSocket) { pSocket->OnDisconnected(); }
    static inline void CallOnRead(StreamSocket *pSocket) { pSocket->OnRead(); }

private:
    // Factory methods.
    static SocketMultiplexer *CreateGenericMultiplexer();
};

template<class T>
ListenSocket *SocketMultiplexer::CreateListenSocket(const SocketAddress *pAddress)
{
    CreateStreamSocketCallback callback = [](SocketMultiplexer *pMultiplexer, StreamSocketImpl *pImpl) -> StreamSocket * { return new T(pMultiplexer, pImpl); }
    return InternalCreateListenSocket(pAddress, callback);
}

template<class T>
T *SocketMultiplexer::ConnectStreamSocket(const SocketAddress *pAddress)
{
    CreateStreamSocketCallback callback = [](SocketMultiplexer *pMultiplexer, StreamSocketImpl *pImpl) -> StreamSocket * { return new T(pMultiplexer, pImpl); }
    return InternalConnectStreamSocket(pAddress, callback);
}

template<class T>
T *SocketMultiplexer::BeginConnectStreamSocket(const SocketAddress *pAddress)
{
    CreateStreamSocketCallback callback = [](SocketMultiplexer *pMultiplexer, StreamSocketImpl *pImpl) -> StreamSocket * { return new T(pMultiplexer, pImpl); }
    return InternalBeginConnectStreamSocket(pAddress, callback);
}

SocketMultiplexer *SocketMultiplexer::Create(SOCKET_MULTIPLEXER_TYPE type /*= NUM_SOCKET_MULTIPLEXER_TYPES*/)
{
    // if set to max, select the best for the platform
    if (type == NUM_SOCKET_MULTIPLEXER_TYPES)
        type = SOCKET_MULTIPLEXER_TYPE_GENERIC;

    // redirect to factory method
    switch (type)
    {
    case SOCKET_MULTIPLEXER_TYPE_GENERIC:
        return CreateGenericMultiplexer();
    }

    // unknown type
    return nullptr;
}

