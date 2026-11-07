#include <net/include/SocketUtil.h>
#include <misc/include/Util.h>
#include <iostream>

UDPSocketPtr SocketUtil::CreateUDPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_DGRAM, IPPROTO_UDP);
	if (s != INVALID_SOCKET)
	{
		return UDPSocketPtr(new UDPSocket(s));
	}
	else
	{
		ReportError(L"SocketUtil::CreateUDPSocket");
		return nullptr;
	}
}

TCPSocketPtr SocketUtil::CreateTCPSocket(SocketAddressFamily inFamily)
{
	SOCKET s = socket(inFamily, SOCK_STREAM, IPPROTO_TCP);
	if (s != INVALID_SOCKET)
	{
		return TCPSocketPtr(new TCPSocket(s));
	}
	else
	{
		ReportError(L"SocketUtil::CreateTCPSocket");
		return nullptr;
	}
}

void SocketUtil::ReportError(PCWSTR str_)
{
	LPCWSTR aString{ str_ };
	std::string aStr{ util::LPCWSTR_to_string(aString) };
	std::cout << aStr;
}

