#include "YBaseLib/Sockets/SocketAddress.h"
#include "YBaseLib/Sockets/SystemHeaders.h"

bool SocketAddress::Parse(Type type, const char *address, uint32 port, SocketAddress *pOutAddress)
{
    switch (type)
    {
    case Type_IPv4:
        {
            sockaddr_in *pSockaddrIn = reinterpret_cast<sockaddr_in *>(pOutAddress->m_data);
            Y_memzero(pSockaddrIn, sizeof(sockaddr_in));
            pSockaddrIn->sin_family = AF_INET;
            pSockaddrIn->sin_port = htons((uint16)port);
            int res = inet_pton(AF_INET, address, &pSockaddrIn->sin_addr);
            if (res != 1)
                return false;
            else
                return true;
        }

    case Type_IPv6:
        {
            sockaddr_in6 *pSockaddrIn = reinterpret_cast<sockaddr_in6 *>(pOutAddress->m_data);
            Y_memzero(pSockaddrIn, sizeof(sockaddr_in6));
            pSockaddrIn->sin6_family = AF_INET;
            pSockaddrIn->sin6_port = htons((uint16)port);
            int res = inet_pton(AF_INET6, address, &pSockaddrIn->sin6_addr);
            if (res != 1)
                return false;
            else
                return true;
        }
    }

    return false;
}

void SocketAddress::ToString(String &destination) const
{
    switch (m_type)
    {
    case Type_IPv4:
        {
            destination.Clear();
            destination.Reserve(128);
            const char *res = inet_ntop(AF_INET, m_data, destination.GetWriteableCharArray(), destination.GetWritableBufferSize());
            if (res == nullptr)
            {
                destination.Clear();
                destination.AppendString("<unknown>");
            }

            destination.AppendFormattedString(":%u", (uint32)ntohs(reinterpret_cast<const sockaddr_in *>(m_data)->sin_port));
            break;
        }

    case Type_IPv6:
        {
            destination.Clear();
            destination.Reserve(128);
            destination.AppendCharacter('[');
            const char *res = inet_ntop(AF_INET, m_data, destination.GetWriteableCharArray() + 1, destination.GetWritableBufferSize() - 1);
            if (res == nullptr)
            {
                destination.Clear();
                destination.AppendString("<unknown>");
            }

            destination.AppendFormattedString("]:%u", (uint32)ntohs(reinterpret_cast<const sockaddr_in6 *>(m_data)->sin6_port));
            break;
        }

    default:
        {
            destination.Clear();
            destination.AppendString("<unknown>");
            break;
        }
    }

}

SmallString SocketAddress::ToString() const
{
    SmallString str;
    ToString(str);
    return str;
}

