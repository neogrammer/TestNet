#include <net/include/SocketAddress.h>
#include <WS2tcpip.h>

SocketAddress::SocketAddress(uint32_t inAddress, uint16_t inPort)
{
	GetAsSockAddrIn()->sin_family = AF_INET;
	GetAsSockAddrIn()->sin_addr.S_un.S_addr = htonl(inAddress);
	GetAsSockAddrIn()->sin_port = htons(inPort);
}

SocketAddress::SocketAddress(PCWSTR ipAddress, uint16_t inPort)
{
	memset(&mSockAddr, 0, sizeof(mSockAddr));
	GetAsSockAddrIn()->sin_family = AF_INET;
	GetAsSockAddrIn()->sin_port = htons(inPort);
	InetPton(AF_INET, ipAddress, &GetAsSockAddrIn()->sin_addr);
}

SocketAddress::SocketAddress(const sockaddr& inSockAddr)
{
	memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
}

SocketAddress::SocketAddress()
{
	GetAsSockAddrIn()->sin_family = AF_INET;
	GetIP4Ref() = INADDR_ANY;
	GetAsSockAddrIn()->sin_port = 0;
}

size_t SocketAddress::GetSize() const { return sizeof(sockaddr); }


sockaddr_in* SocketAddress::GetAsSockAddrIn()
{
	return reinterpret_cast<sockaddr_in*>(&mSockAddr);
}
