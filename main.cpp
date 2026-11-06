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
///
///  Globals
/// 
bool gIsGameRunning = false;

enum SocketAddressFamily
{
	INET = AF_INET,
	INET6 = AF_INET6
};

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

class SocketAddressFactory
{
public:
	static SocketAddressPtr CreateIPv4FromString(const std::string& inStr)
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
};

std::string LPCWSTR_to_string(LPCWSTR lpwsz) {
	int len = WideCharToMultiByte(CP_UTF8, 0, lpwsz, -1, nullptr, 0, nullptr, nullptr);
	if (len <= 0) return {};
	std::string result(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, lpwsz, -1, &result[0], len, nullptr, nullptr);
	return result;
}
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

class SocketUtil
{
public:
	static void ReportError(PCWSTR str_)
	{
		LPCWSTR aString{ str_ };
		std::string aStr{ LPCWSTR_to_string(aString) };
		std::cout << aStr;
	}

	static int GetLastError()
	{
		return WSAGetLastError();
	}

	static UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
	static TCPSocketPtr CreateTCPSocket(SocketAddressFamily inFamily);

};

void DoGameLoop();
void DoServerLoop(UDPSocketPtr sock);

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

	//auto sock =	SocketAddressFactory::CreateIPv4FromString("localhost:55555");
	//auto tcp = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	//auto bound = tcp->Bind(*sock);
	

	while (true)
	{
		std::cout << "Are you the Client or the Server? (C) or (S): ";
		char in;
		std::cin >> in;

		if (in == 'c' || in == 'C')
		{
			std::cout << "\nYou are the client!\n";

			//if (tcp->Connect(*sock) != 0)
			//{
			//	SocketUtil::ReportError(L"\nUnable to connect to server!\n");
			//	break;
			//}
			//system("Pause");
			DoGameLoop();
			break;
		}
		else if (in == 's' || in == 'S')
		{


			std::cout << "\nYou are the server!\n";

			auto udp = SocketUtil::CreateUDPSocket(SocketAddressFamily::INET);
			DoServerLoop(udp);

			//int result = tcp->Listen(32);
			//if (result < 0)
			//{
			//	SocketUtil::ReportError(L"\nListen Failed");
			//	break;
			//}
			//else
			//{
			//	
			//	auto receivingAddress = SocketAddressFactory::CreateIPv4FromString("loopback:55555");
			//	auto tcpClient = tcp->Accept(*receivingAddress);




			//	system("Pause");
			//}

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


//////////////////////////////
//////////////////////////////



SocketAddress::SocketAddress(uint32_t inAddress, uint16_t inPort)
{
	GetAsSockAddrIn()->sin_family = AF_INET;
	GetAsSockAddrIn()->sin_addr.S_un.S_addr = htonl(inAddress);
	GetAsSockAddrIn()->sin_port = htons(inPort);
}

SocketAddress::SocketAddress(PCWSTR ipAddress, uint16_t inPort)
{
	memset(&mSockAddr, 0, sizeof(mSockAddr));
	GetAsSockAddrIn()->sin_family = AF_INET;
	GetAsSockAddrIn()->sin_port = htons(inPort);
	InetPton(AF_INET, ipAddress, &GetAsSockAddrIn()->sin_addr);
}

SocketAddress::SocketAddress(const sockaddr& inSockAddr)
{
	memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
}

SocketAddress::SocketAddress()
{
		GetAsSockAddrIn()->sin_family = AF_INET;
		GetIP4Ref() = INADDR_ANY;
		GetAsSockAddrIn()->sin_port = 0;
}

size_t SocketAddress::GetSize() const { return sizeof(sockaddr); }


sockaddr_in* SocketAddress::GetAsSockAddrIn()
{
	return reinterpret_cast<sockaddr_in*>(&mSockAddr);
}



/// //////////////////////////////////////////////////////
/// //////////////////////////////////////////////////

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

/// //////////////////////////////////////////////////////////////
/// ///////////////////////////////////////////////////////////////


int UDPSocket::Bind(const SocketAddress& inBindAddress)
{
	int err = bind(mSocket, &inBindAddress.mSockAddr, static_cast<int>(inBindAddress.GetSize()));
	if (err != 0)
	{

		SocketUtil::ReportError(L"UDPSocket::Bind");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

int UDPSocket::ReceiveFrom(void* inBuffer, int inLen, int inMaxLength, SocketAddress& outFrom)
{
	int fromLength = static_cast<int>(outFrom.GetSize());
	

	
	int readyByteCount = recvfrom(mSocket, static_cast<char*>(inBuffer), inMaxLength, 0, &outFrom.mSockAddr, &fromLength);
	if (readyByteCount >= 0) return readyByteCount;

	int err = WSAGetLastError();
	if (err == WSAEWOULDBLOCK) return 0;      // nothing waiting, not an error
	SocketUtil::ReportError(L"UDPSocket::ReceiveFrom");
	return -err;
}

int UDPSocket::SetNonBlockingMode(bool inShouldBeNonBlocking)
{
#if _WIN32
	u_long arg = inShouldBeNonBlocking ? 1 : 0;
	int result = ioctlsocket(mSocket, FIONBIO, &arg);
#else
	int flags = fcntl(mSocket, F_GETFL, 0);
	flags = inShouldBeNonBlocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
	fcntl(mSocket, F_SETFL, flags);
#endif

	if (result == SOCKET_ERROR)
	{
		SocketUtil::ReportError(L"UDPSocket::SetNonBlockingMode");
		return SocketUtil::GetLastError();
	}
	else
	{
		return NO_ERROR;
	}
} 

int UDPSocket::SendTo(const void* inData, int inLen, const SocketAddress& inTo)
{
	int byteSentCount = sendto(mSocket, static_cast<const char*>(inData), inLen, 0, &(inTo.mSockAddr), static_cast<int>(inTo.GetSize()));
	if (byteSentCount >= 0)
	{
		return byteSentCount;
	}
	else
	{
		// return error
		SocketUtil::ReportError(L"UDPSocket::SendTo");
		return -SocketUtil::GetLastError();
	}
}

UDPSocket::~UDPSocket()
{
	closesocket(mSocket);
}

//////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
  

TCPSocket::~TCPSocket()
{
	closesocket(mSocket);
}

int TCPSocket::Connect(const SocketAddress& inAddress)
{
	int err = connect(mSocket, &inAddress.mSockAddr, static_cast<int>(inAddress.GetSize()));
	if (err < 0)
	{
		SocketUtil::ReportError(L"TCPSocket::Connect");
		return -SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

int TCPSocket::Bind(const SocketAddress& inToAddress)
{
	int err = bind(mSocket, &inToAddress.mSockAddr, static_cast<int>(inToAddress.GetSize()));
	if (err != 0)
	{
		SocketUtil::ReportError(L"TCPSocket::Bind");
		return SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

int TCPSocket::Listen(int inBackLog)
{
	int err = listen(mSocket, inBackLog);
	if (err < 0)
	{
		SocketUtil::ReportError(L"TCPSocket::Listen");
		return -SocketUtil::GetLastError();
	}
	return NO_ERROR;
}

std::shared_ptr<TCPSocket> TCPSocket::Accept(SocketAddress& inFromAddress)
{
	int length = static_cast<int>(inFromAddress.GetSize());
	SOCKET newSocket = accept(mSocket, &inFromAddress.mSockAddr, &length);

	if (newSocket != INVALID_SOCKET)
	{
		return TCPSocketPtr(new TCPSocket(newSocket));
	}
	else
	{
		SocketUtil::ReportError(L"TCPSocket::Accept");
		return nullptr;
	}

}

int TCPSocket::Send(const void* inData, int inLen)
{
	int bytesSentCount = send(mSocket, static_cast<const char*>(inData), inLen, 0);

	if (bytesSentCount < 0)
	{
		SocketUtil::ReportError(L"TCPSocket::Send");
		return -SocketUtil::GetLastError();
	}
	return bytesSentCount;
}

int TCPSocket::Receive(void* inBuffer, int inLen)
{
	int bytesReceivedCount = recv(mSocket, static_cast<char*>(inBuffer), inLen, 0);
	if (bytesReceivedCount < 0)
	{
		SocketUtil::ReportError(L"TCPSocket::Receive");
		return -SocketUtil::GetLastError();
	}

	return bytesReceivedCount;
}

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
