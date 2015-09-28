#include "YBaseLib/Sockets/StreamSocket.h"
#include "YBaseLib/Sockets/StreamSocketImpl.h"
#include "YBaseLib/Error.h"
#include "YBaseLib/Log.h"
Log_SetChannel(StreamSocket);

StreamSocket::StreamSocket(SocketMultiplexer *pMultiplexer, StreamSocketImpl *pImpl)
    : m_pMultiplexer(pMultiplexer)
    , m_pImpl(pImpl)
    , m_connected(false)
{

}

StreamSocket::~StreamSocket()
{
    delete m_pImpl;
}

size_t StreamSocket::Read(void *pBuffer, size_t bufferSize)
{
    if (!m_connected)
        return 0;

    size_t receievedBytes;
    Error error;
    if (!m_pImpl->Read(pBuffer, &receievedBytes, &error))
    {
        Log_ErrorPrintf("Read error: %s", error.GetErrorCodeAndDescription().GetCharArray());
        OnDisconnected();
        return 0;
    }

    return receievedBytes;
}

size_t StreamSocket::Write(const void *pBuffer, size_t bufferLength)
{
    if (!m_connected)
        return 0;

    size_t writtenBytes;
    Error error;
    if (!m_pImpl->Write(pBuffer, &writtenBytes, &error))
    {
        Log_ErrorPrintf("Write error: %s", error.GetErrorCodeAndDescription().GetCharArray());
        OnDisconnected();
        return 0;
    }

    return writtenBytes;
}

void StreamSocket::Close()
{
    if (!m_connected)
        return;

    m_pImpl->Close();
    OnDisconnected();
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

