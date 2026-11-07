#include <net/include/UDPSocket.h>
#include <net/include/SocketUtil.h>


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