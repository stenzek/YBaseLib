#include "YBaseLib/Sockets/BaseSocket.h"
#include "YBaseLib/Sockets/StreamSocketImpl.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
Log_SetChannel(StreamSocket);

BaseSocket::BaseSocket(SocketMultiplexer *pMultiplexer)
    : m_pMultiplexer(pMultiplexer)
{

}

BaseSocket::~BaseSocket()
{

}
