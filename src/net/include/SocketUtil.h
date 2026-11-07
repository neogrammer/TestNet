#pragma once
#include <net/include/TCPSocket.h>
#include <net/include/UDPSocket.h>
#include <net/include/SocketAddressFamily.h>
#include <Windows.h>

static class SocketUtil
{
public:
	static void ReportError(PCWSTR str_);


	static int GetLastError()
	{
		return WSAGetLastError();
	}

	static UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
	static TCPSocketPtr CreateTCPSocket(SocketAddressFamily inFamily);
};
