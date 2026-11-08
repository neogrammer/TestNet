#pragma once
#include <net/include/TCPSocket.h>
#include <net/include/UDPSocket.h>
#include <net/include/SocketAddressFamily.h>
#include <Windows.h>
#include <vector>

class SocketUtil
{
public:
	static void ReportError(PCWSTR str_);


	static int GetLastError()
	{
		return WSAGetLastError();
	}

	static UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
	static TCPSocketPtr CreateTCPSocket(SocketAddressFamily inFamily);

	static fd_set* FillSetFromVector(fd_set& outSet, const std::vector<TCPSocketPtr>* inSockets);
	static void FillVectorFromSet(std::vector<TCPSocketPtr>* outSockets, const std::vector<TCPSocketPtr>* inSockets, const fd_set& inSet);
    static int Select(const std::vector<TCPSocketPtr>* inReadSet, std::vector<TCPSocketPtr>* outReadSet, const std::vector<TCPSocketPtr>* inWriteSet, std::vector<TCPSocketPtr>* outWriteSet, const std::vector<TCPSocketPtr>* inExceptSet, std::vector<TCPSocketPtr>* outExceptSet);
};
