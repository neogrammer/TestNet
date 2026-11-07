#pragma once
#include <WinSock2.h>
#include <memory>
#include <utility>
#include <net/include/SocketAddress.h>
#include <net/include/TCPSocket.h>

class TCPSocket
{
	friend class SocketUtil;
	TCPSocket(SOCKET inSocket) :mSocket(inSocket) {}
	SOCKET mSocket;

public:
	~TCPSocket();
	int Connect(const SocketAddress& inAddress);
	int Bind(const SocketAddress& inToAddress);
	int Listen(int inBackLog = 32);
	std::shared_ptr<TCPSocket> Accept(SocketAddress& inFromAddress);
	int Send(const void* inData, int inLen);
	int Receive(void* inBuffer, int inLen);

};
typedef std::shared_ptr<TCPSocket> TCPSocketPtr;