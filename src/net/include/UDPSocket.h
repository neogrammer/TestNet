#pragma once
#include <WinSock2.h>
#include <memory>
#include <utility>
#include <net/include/SocketAddress.h>
#include <net/include/UDPSocket.h>

class UDPSocket
{
public:
	~UDPSocket();
	int Bind(const SocketAddress& inToAddress);
	int SendTo(const void* inData, int inLen, const SocketAddress& inTo);
	int ReceiveFrom(void* inBuffer, int inLen, int inMaxLength, SocketAddress& outFrom);
	int SetNonBlockingMode(bool inShouldBeNonBlocking);

	friend class SocketUtil;
	UDPSocket(SOCKET inSocket) : mSocket(inSocket) {

	}
private:
	SOCKET mSocket;
};
typedef std::shared_ptr<UDPSocket> UDPSocketPtr;