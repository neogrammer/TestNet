#pragma once
#include <string>
#include <Winsock2.h>
#include <net/include/SocketAddress.h>


class SocketAddressFactory
{
public:
	static SocketAddressPtr CreateIPv4FromString(const std::string& inStr);
};