#pragma once
#include "YBaseLib/Common.h"
#include "YBaseLib/String.h"

struct SocketAddress
{
    enum Type
    {
        Type_IPv4,
        Type_IPv6,
        Type_NamedPipe,
        Type_SharedMemory
    };

    static bool Parse(Type type, const char *address, uint32 port, SocketAddress *pOutAddress);

    void ToString(String &destination) const;
    SmallString ToString() const;

private:
    Type m_type;
    byte m_data[128];
};
