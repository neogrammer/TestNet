#include <net/include/SocketAddressFactory.h>
#include <Ws2tcpip.h>

SocketAddressPtr SocketAddressFactory::CreateIPv4FromString(const std::string& inStr)
{
	auto pos = inStr.find_last_of(':');
	std::string host, service;
	if (pos != std::string::npos)
	{
		host = inStr.substr(0, pos);
		service = inStr.substr(pos + 1);
	}
	else
	{
		host = inStr;
		service = "0";
	}
	addrinfo hint;
	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_INET;

	addrinfo* result;
	int error = getaddrinfo(host.c_str(), service.c_str(), &hint, &result);

	if (error != 0 && result != nullptr)
	{
		freeaddrinfo(result);
		return nullptr;
	}

	while (!result->ai_addr && result->ai_next)
	{
		result = result->ai_next;
	}

	if (!result->ai_addr)
	{
		freeaddrinfo(result);
		return nullptr;
	}

	auto toRet = std::make_shared<SocketAddress>(*result->ai_addr);

	freeaddrinfo(result);

	return toRet;
}
