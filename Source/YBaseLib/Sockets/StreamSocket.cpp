#include "YBaseLib/Sockets/StreamSocket.h"

StreamSocket::StreamSocket(StreamSocketTransport *pTransport /*= nullptr*/)
    : m_pTransport(pTransport)
    , m_connected(false)
{

}

StreamSocket::~StreamSocket()
{
    delete m_pTransport;
}

bool StreamSocket::Connect(const SocketAddress *pAddress)
{
    return false;
}

bool StreamSocket::BeginConnect(const SocketAddress *pAddress)
{
    return false;
}

uint32 StreamSocket::Send(const void *pBuffer, size_t bufferLength)
{
    if (!m_connected)
        return 0;

    return 0;
}

void StreamSocket::OnConnected()
{
    m_connected = true;
}

void StreamSocket::OnDisconnected()
{
    m_connected = false;
}

void StreamSocket::OnRead()
{

}

