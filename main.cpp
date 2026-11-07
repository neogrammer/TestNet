#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include <Ws2Tcpip.h>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include <thread>
#include <chrono>
#include <net/include/SocketAddressFamily.h>
#include <net/include/SocketAddress.h>
#include  <net/include/SocketUtil.h>
#include <net/include/UDPSocket.h>
#include  <net/include/TCPSocket.h>
#include <net/include/SocketAddressFactory.h>
#include <misc/include/Util.h>

bool gIsGameRunning = false;

void DoGameLoop();
void DoServerLoop(UDPSocketPtr sock);
int ProcessReceivedData(char* data, int bytesReceived, SocketAddress socketAddress);

int main()
{
	HWND myWindow = GetConsoleWindow();
	{
		WSADATA wsaData;
		int code = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (code != 0)
		{
			int error = WSAGetLastError();
			return error;
		}
	}
	while (true)
	{
		std::cout << "Are you the Client or the Server? (C) or (S): ";
		char in;
		std::cin >> in;
		if (in == 'c' || in == 'C')
		{
			std::cout << "\nYou are the client!\n";
			DoGameLoop();
			break;
		}
		else if (in == 's' || in == 'S')
		{
			std::cout << "\nYou are the server!\n";
			auto udp = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET);
			DoServerLoop(udp);
			break;
		}
		else
		{
			std::cout << "\nChoice not good enough, Try again\n";
			continue;
		}
	}
	int err = WSAGetLastError();
	int code = WSACleanup();
	if (code != 0)
	{
		return err;
	}
	return 0;
}

//////////////////////////////////////////////////////////////
//  END OF MAIN
/////////////////////////////////////////////////////////////

void DoGameLoop()
{
	UDPSocketPtr mySock = SocketUtil::CreateUDPSocket(INET);
	SocketAddress listenAddr(INADDR_LOOPBACK, 55555);     // 127.0.0.1:55555
	if (mySock->Bind(listenAddr) != NO_ERROR) return;
	mySock->SetNonBlockingMode(true);
	gIsGameRunning = true;
	while (gIsGameRunning)
	{
		char data[1500];
		SocketAddress socketAddress{};
		int bytesReceived = mySock->ReceiveFrom(data, sizeof(data), 1500, socketAddress);
		if (bytesReceived > 0)
		{
			int left = ProcessReceivedData(data, bytesReceived, socketAddress);
			if (left == 0)	
			{
				std::cout << "All data received!" << std::endl;
			}
			else
			{
				std::cout << "Bytes left to receive: " << left << std::endl;
			}
		}
	}
}
void DoServerLoop(UDPSocketPtr sock)
{
	SocketAddress clientAddr(INADDR_LOOPBACK, 55555);
	std::string data = { "IHATEMYLIFESOOMUCHIJUSTWANTTOFUCKASHEEP" };
	gIsGameRunning = true;
	//sock->SetNonBlockingMode(true);
	while (gIsGameRunning)
	{
		sock->SendTo(data.c_str(), static_cast<int>(data.length()), clientAddr);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));

	}
}
int ProcessReceivedData(char* data, int bytesReceived, SocketAddress socketAddress)
{
	static int bytesTotal{ 0 };
	bytesTotal += bytesReceived;
	const std::string strTotal = "IHATEMYLIFESOOMUCHIJUSTWANTTOFUCKASHEEP";
	std::cout << "Received " << bytesReceived << " plus buffer is a total of " << bytesTotal << " of " << strTotal.length() << std::endl;
	if (bytesTotal >= strTotal.length())
	{
		std::cout << "All data received!" << std::endl;
		bytesTotal = 0;
		return 0;
	}
	else
	{
		std::cout << "Bytes left to receive: " << strTotal.length() - bytesTotal << std::endl;
		return  static_cast<int>(strTotal.length()) - bytesTotal;
	}
}
