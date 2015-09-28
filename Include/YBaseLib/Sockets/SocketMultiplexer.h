#pragma once
#include "YBaseLib/Common.h"

class SocketAddress;
class ListenSocket;
class StreamSocket;
class StreamSocketTransport;
class DatagramSocket;
class DatagramSocketImpl;

enum SOCKET_MULTIPLEXER_TYPE
{
    SOCKET_MULTIPLEXER_TYPE_GENERIC,
    //SOCKET_MULTIPLEXER_TYPE_EPOLL,
    //SOCKET_MULTIPLEXER_TYPE_KQUEUE,
    //SOCKET_MULTIPLEXER_TYPE_IOCP,
    NUM_SOCKET_MULTIPLEXER_TYPES
};

class SocketMultiplexer
{
public:
    SocketMultiplexer();
    virtual ~SocketMultiplexer();

    virtual SOCKET_MULTIPLEXER_TYPE GetType() const = 0;

    virtual StreamSocket *ConnectStreamSocket(const SocketAddress *pAddress) = 0;
    virtual StreamSocket *BeginConnectStreamSocket(const SocketAddress *pAddress) = 0;
    virtual ListenSocket *Create


protected:
    virtual void AddSocket(TCPSocket *pSocket) = 0;
    virtual void AddSocket(UDPSocket *pSocket) = 0;
    virtual void 
};