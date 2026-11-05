#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Winsock2.h>
#include <Ws2Tcpip.h>
#include <memory>
#include <string>
#include <utility>
#include <iostream>
#include <thread>

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
	size_t GetSize() const;
	sockaddr mSockAddr;
private:
	sockaddr_in* GetAsSockAddrIn();
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

	UDPSocketPtr CreateUDPSocket(SocketAddressFamily inFamily);
	static TCPSocketPtr CreateTCPSocket(SocketAddressFamily inFamily);

};



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

	auto sock =	SocketAddressFactory::CreateIPv4FromString("localhost:55555");
	auto tcp = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);
	auto bound = tcp->Bind(*sock);
	

	while (true)
	{
		std::cout << "Are you the Client or the Server? (C) or (S): ";
		char in;
		std::cin >> in;

		if (in == 'c' || in == 'C')
		{
			std::cout << "\nYou are the client!\n";

			if (tcp->Connect(*sock) != 0)
			{
				SocketUtil::ReportError(L"\nUnable to connect to server!\n");
				break;
			}
			system("Pause");

			break;
		}
		else if (in == 's' || in == 'S')
		{


			std::cout << "\nYou are the server!\n";

			int result = tcp->Listen(32);
			if (result < 0)
			{
				SocketUtil::ReportError(L"\nListen Failed");
				break;
			}
			else
			{
				sockaddr tmp{};
				SocketAddress receivingAddress{tmp};
				auto tcpClient = tcp->Accept(receivingAddress);

				system("Pause");
			}

			break;
		}
		else
		{
			std::cout << "\nChoice not good enough, Try again\n";
			continue;
		}
	}
	int code = WSACleanup();



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
	sockaddr_in myAddr;
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(inPort);
#ifdef _WIN32
	InetPton(AF_INET, ipAddress, &myAddr.sin_addr);
#else
	inet_pton(AF_INET, PCTSTR(&myAddr.sin_addr));
#endif
}

SocketAddress::SocketAddress(const sockaddr& inSockAddr)
{
	memcpy(&mSockAddr, &inSockAddr, sizeof(sockaddr));
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
	int readyByteCount = recvfrom(mSocket, static_cast<char*>(inBuffer), inMaxLength, 0, &outFrom.mSockAddr, &inLen);


	if (readyByteCount >= 0)
	{
		return readyByteCount;
	}
	else
	{
		SocketUtil::ReportError(L"UDPSocket::ReceiveFrom");
		return -SocketUtil::GetLastError();
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
 