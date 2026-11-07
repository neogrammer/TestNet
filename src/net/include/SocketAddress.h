#pragma once
#include <Winsock2.h>
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <utility>

class SocketAddress
{
public:

	explicit SocketAddress(uint32_t inAddress, uint16_t inPort);
	explicit SocketAddress(PCWSTR ipAddress, uint16_t inPort);

	explicit SocketAddress(const sockaddr& inSockAddr);
	explicit SocketAddress();
	size_t GetSize() const;
	sockaddr mSockAddr;

private:

#if _WIN32
	uint32_t& GetIP4Ref() { return *reinterpret_cast<uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }
	const uint32_t& GetIP4Ref()			const { return *reinterpret_cast<const uint32_t*>(&GetAsSockAddrIn()->sin_addr.S_un.S_addr); }
#else
	uint32_t& GetIP4Ref() { return GetAsSockAddrIn()->sin_addr.s_addr; }
	const uint32_t& GetIP4Ref()			const { return GetAsSockAddrIn()->sin_addr.s_addr; }
#endif
public:
	sockaddr_in* GetAsSockAddrIn();
	const	sockaddr_in* GetAsSockAddrIn()	const { return reinterpret_cast<const sockaddr_in*>(&mSockAddr); }
};
typedef std::shared_ptr<SocketAddress> SocketAddressPtr;