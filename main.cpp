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
#define GOOD_SEGMENT_SIZE 1300

bool gIsGameRunning = false;

void DoGameLoop();
void DoServerLoop(UDPSocketPtr sock);
void DoTCPLoop();
int ProcessReceivedData(char* data, int bytesReceived, SocketAddress socketAddress);
void ProcessNewClient(TCPSocketPtr socket, SocketAddress socketAddress);
void ProcessDataFromClient(TCPSocketPtr socket, char* segment, int dataReceived);

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
			DoTCPLoop();
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
TCPSocketPtr toServerSocket;
void DoGameLoop()
{
	//UDPSocketPtr mySock = SocketUtil::CreateUDPSocket(INET);
	//SocketAddress listenAddr(INADDR_LOOPBACK, 55555);     // 127.0.0.1:55555
	//if (mySock->Bind(listenAddr) != NO_ERROR) return;
	//mySock->SetNonBlockingMode(true);
	//gIsGameRunning = true;
	//while (gIsGameRunning)
	//{
	//	char data[1500];
	//	SocketAddress socketAddress{};
	//	int bytesReceived = mySock->ReceiveFrom(data, sizeof(data), 1500, socketAddress);
	//	if (bytesReceived > 0)
	//	{
	//		int left = ProcessReceivedData(data, bytesReceived, socketAddress);
	//		if (left == 0)	
	//		{
	//			std::cout << "All data received!" << std::endl;
	//		}
	//		else
	//		{
	//			std::cout << "Bytes left to receive: " << left << std::endl;
	//		}
	//	}
	//}
	toServerSocket = SocketUtil::CreateTCPSocket(INET);
	SocketAddress serverAddr(INADDR_LOOPBACK, 48000);
	if (toServerSocket->Connect(serverAddr) != NO_ERROR)
	{
		std::cout << "Connect failed. Is the server running?" << std::endl;
		return;
	}

	std::cout << "Successful connection!" << std::endl;
	gIsGameRunning = true;

	while (gIsGameRunning)
	{
		std::string data = { "IHATEMYLIFESOOMUCHIJUSTWANTTOFUCKASHEEP" };
		toServerSocket->Send(data.c_str(), static_cast<int>(data.length()));
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		static int count = 0;
		count++;
		if (count == 12)
		{
			gIsGameRunning = false;
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

std::vector<TCPSocketPtr> readBlockSockets{};
void DoTCPLoop() {
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(INET);
	SocketAddress receivingAddress(INADDR_ANY, 48000);
	if (listenSocket->Bind(receivingAddress) != NO_ERROR)
	{
		std::cout << "Failed to bind to port 48000" << std::endl;
		return;
	}
	readBlockSockets.clear();
	readBlockSockets.push_back(listenSocket);

	if (listenSocket->Listen() != NO_ERROR)
	{
		std::cout << "Failed to listen on port 48000" << std::endl;
		return;
	}
	std::cout << "Listening on port 48000" << std::endl;

	std::vector<TCPSocketPtr> readableSockets;
	gIsGameRunning = true;
	while (gIsGameRunning)
	{
		if (SocketUtil::Select(&readBlockSockets, &readableSockets, nullptr, nullptr, nullptr, nullptr))
		{
			for (const TCPSocketPtr& socket : readableSockets)
			{
				if (socket == listenSocket)
				{
					SocketAddress newClientAddress;
					auto newSocket = listenSocket->Accept(newClientAddress);
					readBlockSockets.push_back(newSocket);
					ProcessNewClient(newSocket, newClientAddress);
				}
				else
				{
					char segment[GOOD_SEGMENT_SIZE];
					int dataReceived = socket->Receive(segment, GOOD_SEGMENT_SIZE);
					if (dataReceived > 0)
					{
						ProcessDataFromClient(socket, segment, dataReceived);
						static int count = 0;
						count++;
						if (count >= 12)
						{
							gIsGameRunning = false;
						}
					}
				}
			}
		}
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

void ProcessNewClient(TCPSocketPtr socket, SocketAddress socketAddress)
{
	
}

void ProcessDataFromClient(TCPSocketPtr socket, char* segment, int dataReceived)
{
	
	const std::string strTotal = std::string(segment, dataReceived);
	std::cout << "Received " << strTotal << std::endl;
}
