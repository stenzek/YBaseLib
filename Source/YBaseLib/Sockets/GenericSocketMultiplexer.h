#pragma once
#include "YBaseLib/Sockets/SocketMultiplexer.h"
#include "YBaseLib/MemArray.h"
#include "YBaseLib/Mutex.h"

class GenericSocketMultiplexer : public SocketMultiplexer
{
public:
    // Listen socket implementation
    class GenericListenSocket : public ListenSocket
    {
    public:
        GenericListenSocket(GenericSocketMultiplexer *pMultiplexer, CreateStreamSocketCallback acceptCallback, const SocketAddress *pAddress, int fileDescriptor);
        virtual ~GenericListenSocket();

        virtual const SocketAddress *GetLocalAddress() const override { return &m_localAddress; }
        virtual uint32 GetConnectionsAccepted() const override { return m_numConnectionsAccepted; }
        virtual void Close() override final;

    protected:
        virtual void OnRead() override final;

    protected:
        GenericSocketMultiplexer *m_pMultiplexer;
        CreateStreamSocketCallback m_acceptCallback;
        SocketAddress m_localAddress;
        uint32 m_numConnectionsAccepted;
        int m_fileDescriptor;
    };

    // Stream socket implementation
    class GenericStreamSocketImpl : public StreamSocketImpl
    {
    public:
        GenericStreamSocketImpl(GenericSocketMultiplexer *pMultiplexer, const SocketAddress *pLocalAddress, const SocketAddress *pRemoteAddress, int fileDescriptor);
        virtual ~GenericStreamSocketImpl();

        virtual const SocketAddress *GetLocalAddress() override final { return &m_localAddress; }
        virtual const SocketAddress *GetRemoteAddress() override final { return &m_remoteAddress; }

        virtual bool Read(void *pBuffer, size_t *pByteCount, Error *pError) override final;
        virtual bool Write(const void *pBuffer, size_t *pByteCount, Error *pError) override final;

        virtual void Close() override final;

    protected:
        GenericSocketMultiplexer *m_pMultiplexer;
        SocketAddress m_localAddress;
        SocketAddress m_remoteAddress;
        uint32 m_lastError;
        int m_fileDescriptor;
    };

public:
    GenericSocketMultiplexer();
    virtual ~GenericSocketMultiplexer();

    virtual SOCKET_MULTIPLEXER_TYPE GetType() const override { return SOCKET_MULTIPLEXER_TYPE_GENERIC; }

    virtual void PollEvents(uint32 milliseconds) override;

    virtual ListenSocket *InternalCreateListenSocket(const SocketAddress *pAddress, CreateStreamSocketCallback acceptCallback) override;
    virtual StreamSocketImpl *InternalConnectStreamSocket(const SocketAddress *pConnect, CreateStreamSocketCallback acceptCallback) override;

private:
    // We store the fd in the struct to avoid the cache miss reading the object.
    struct BoundListenSocket { ListenSocket *pSocket; int FileDescriptor; };
    struct BoundStreamSocket { StreamSocket *pSocket; int FileDescriptor; };
    MemArray<BoundListenSocket> m_boundListenSockets;
    MemArray<BoundStreamSocket> m_boundStreamSockets;
    Mutex m_boundSocketLock;
};

