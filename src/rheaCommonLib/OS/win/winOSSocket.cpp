#ifdef WIN32
#include "../OS.h"
#include "winOSSocket.h"
#include <ws2tcpip.h>
#include "../../rhea.h"


//*************************************************
void platform::socket_init (OSSocket *sok)
{
	assert(sok != NULL);
	sok->socketID = INVALID_SOCKET;
	sok->readTimeoutMSec = 10000;
	//sok->hEventNotify = INVALID_HANDLE_VALUE;
}

//*************************************************
void platform::socket_close (OSSocket &sok)
{
	if (platform::socket_isOpen(sok))
	{
		closesocket(sok.socketID);
		sok.socketID = INVALID_SOCKET;
	}
}

//*************************************************
bool setBlockingMode (OSSocket &sok, bool bBlocking)
{
	u_long iMode = 0;
	if (!bBlocking)
		iMode = 1;

	if (ioctlsocket(sok.socketID, FIONBIO, &iMode) != 0)
	{
		//printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return false;
	}
	return true;
}

//*************************************************
eSocketError socket_openAsTCP(OSSocket *sok)
{
	platform::socket_init(sok);

	//creo la socket
	sok->socketID = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sok->socketID == INVALID_SOCKET)
	{
		switch (WSAGetLastError())
		{
		case EACCES:            return eSocketError_denied;

		case EPROTONOSUPPORT:
		case EINVAL:
		case EAFNOSUPPORT:      return eSocketError_unsupported;

		case EMFILE:
		case ENFILE:            return eSocketError_tooMany;

		case ENOBUFS:
		case ENOMEM:            return eSocketError_noMem;

		default:                return eSocketError_unknown;
		}
	}

	//abilito delle opzioni di socket
	BOOL enable = TRUE;
	setsockopt(sok->socketID, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(BOOL));
	//setsockopt(sok->socketID, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(BOOL));

	return eSocketError_none;
}

//*************************************************
eSocketError platform::socket_openAsTCPClient(OSSocket *sok, const char *connectToIP, u32 portNumber)
{
	eSocketError sokErr = socket_openAsTCP(sok);
	if (sokErr != eSocketError_none)
		return sokErr;

	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(connectToIP);
	clientService.sin_port = htons(portNumber);


	if (0 != connect(sok->socketID, (SOCKADDR *)& clientService, sizeof(clientService)))
	{
		::closesocket(sok->socketID);
		sok->socketID = -1;

		int err = errno;
		switch (errno)
		{
		case EACCES:
		case EPERM:         return eSocketError_addressProtected;
		case EADDRINUSE:    return eSocketError_addressInUse;
		case EINVAL:        return eSocketError_alreadyBound;
		case ENOTSOCK:      return eSocketError_invalidDescriptor;
		case ENOMEM:        return eSocketError_noMem;
		case ECONNREFUSED:  return eSocketError_connRefused;
		case ETIMEDOUT:     return eSocketError_timedOut;
		default:            return eSocketError_unknown;
		}
	}

	return eSocketError_none;
}

//*************************************************
eSocketError platform::socket_openAsTCPServer (OSSocket *sok, int portNumber)
{
	eSocketError sokErr = socket_openAsTCP(sok);
	if (sokErr != eSocketError_none)
		return sokErr;


	//blocking
	setBlockingMode(*sok, true);

	//la bindo
	struct sockaddr_in socket_config;
	socket_config.sin_family = AF_INET;
	socket_config.sin_addr.s_addr = INADDR_ANY;
	socket_config.sin_port = htons(portNumber);

	if (0 != bind(sok->socketID, (struct sockaddr *)&socket_config, sizeof(socket_config)))
	{
		::closesocket(sok->socketID);
		sok->socketID = INVALID_SOCKET;
		switch (errno)
		{
		case EACCES:        return eSocketError_addressProtected;
		case EADDRINUSE:    return eSocketError_addressInUse;
		case EINVAL:        return eSocketError_alreadyBound;
		case ENOTSOCK:      return eSocketError_invalidDescriptor;
		case ENOMEM:        return eSocketError_noMem;
		default:            return eSocketError_unknown;
		}
	}


	//read timeout di default
	platform::socket_setReadTimeoutMSec(*sok, sok->readTimeoutMSec);

	//tutto ok
	return eSocketError_none;
}


//*************************************************
bool platform::socket_setReadTimeoutMSec  (OSSocket &sok, u32 timeoutMSecIN)
{
	if (!platform::socket_isOpen(sok))
		return false;

	DWORD timeoutMSec = timeoutMSecIN;
	if (timeoutMSecIN == u32MAX)
		timeoutMSec = 0;    //socket sempre bloccante
	else if (timeoutMSec == 0)
		timeoutMSec = 1;    //socket con il minimo possibile tempo di wait

	if (0 == setsockopt (sok.socketID, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeoutMSec, sizeof(DWORD)))
	{
		sok.readTimeoutMSec = timeoutMSecIN;
		return true;
	}

	/*switch (errno)
	{
	case EBADF:         printf ("Error code=EBADF\n"); break;
	case EFAULT:        printf ("Error code=EFAULT\n"); break;
	case EINVAL:        printf ("Error code=EINVAL\n"); break;
	case ENOPROTOOPT:   printf ("Error code=ENOPROTOOPT\n"); break;
	case ENOTSOCK:      printf ("Error code=ENOTSOCK\n"); break;

	default:
		printf ("Error code=%d\n", errno);
		break;
	}
	*/
	return false;
}

//*************************************************
bool platform::socket_setWriteTimeoutMSec (OSSocket &sok, u32 timeoutMSecIN)
{
	if (!platform::socket_isOpen(sok))
		return false;
	
	DWORD timeoutMSec = (DWORD)timeoutMSecIN;
	if (timeoutMSecIN == u32MAX)
		timeoutMSec = 0;    //socket sempre bloccante
	else if (timeoutMSec == 0)
		timeoutMSec = 1;    //socket con il minimo possibile tempo di wait

	
	if (0 == setsockopt(sok.socketID, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeoutMSec, sizeof(DWORD)))
		return true;

	/*switch (errno)
	{
	case EBADF:         printf ("Error code=EBADF\n"); break;
	case EFAULT:        printf ("Error code=EFAULT\n"); break;
	case EINVAL:        printf ("Error code=EINVAL\n"); break;
	case ENOPROTOOPT:   printf ("Error code=ENOPROTOOPT\n"); break;
	case ENOTSOCK:      printf ("Error code=ENOTSOCK\n"); break;

	default:
		printf ("Error code=%d\n", errno);
		break;
	}
	*/
	return false;
}

//*************************************************
bool platform::socket_listen (const OSSocket &sok, u16 maxIncomingConnectionQueueLength)
{
	if (!platform::socket_isOpen(sok))
		return false;

	int err;
	if (u16MAX == maxIncomingConnectionQueueLength)
		err = ::listen(sok.socketID, SOMAXCONN);
	else
		err = ::listen(sok.socketID, (int)maxIncomingConnectionQueueLength);

	return (err == 0);
}

//*************************************************
bool platform::socket_accept (const OSSocket &sok, OSSocket *out_clientSocket)
{
	if (!platform::socket_isOpen(sok))
		return false;

	socket_init(out_clientSocket);

	int clientSocketID = ::accept(sok.socketID, NULL, NULL);
	if (clientSocketID == INVALID_SOCKET)
		return false;

	out_clientSocket->socketID = clientSocketID;
	setBlockingMode(*out_clientSocket, true);
	platform::socket_setReadTimeoutMSec(*out_clientSocket, out_clientSocket->readTimeoutMSec);
	return true;
}


//*************************************************
i32 platform::socket_read(OSSocket &sok, void *buffer, u16 bufferSizeInBytes, u32 timeoutMSec)
{
	if (timeoutMSec != sok.readTimeoutMSec)
		platform::socket_setReadTimeoutMSec(sok, timeoutMSec);
		

	//purtroppo devo pollare.... in teoria socket_setReadTimeoutMSec dovrebbe settare
	//la socket in modalità bloccante con il timeout deciso, ma di fatto non funziona quindi mi tocca pollare
	u64 timeToExitMSec = OS_getTimeNowMSec() + timeoutMSec;
	i32 ret = -1;
	do
	{
		ret = ::recv(sok.socketID, (char*)buffer, bufferSizeInBytes, 0);
		if (ret == 0)
			return 0;	//socket closed
		if (ret != SOCKET_ERROR)
			return ret;

		assert(ret == SOCKET_ERROR);
		int myerrno = WSAGetLastError();
		if (myerrno == WSAEINPROGRESS || myerrno == WSAEWOULDBLOCK || myerrno == WSAETIMEDOUT)
		{
			OS_sleepMSec(100);
			continue;
		}

		switch (myerrno)
		{
		case WSANOTINITIALISED:
			return 0;
		case WSAENETDOWN:
			return 0;
		case WSAEFAULT:
			return 0;
		case WSAENOTCONN:
			return 0;
		case WSAEINTR:
			return 0;
		case WSAENETRESET:
			return 0;
		case WSAENOTSOCK:
			return 0;
		case WSAEOPNOTSUPP:
			return 0;
		case MSG_OOB:
			return 0;
		case WSAESHUTDOWN:
			return 0;
		case WSAEMSGSIZE:
			return 0;
		
		case WSAECONNRESET:
			return 0;

		case WSAEINVAL:
			return 0;

		case WSAECONNABORTED:
			return 0;


		}
	} while (OS_getTimeNowMSec() < timeToExitMSec);

	return -1;  //timeout
}

//*************************************************
i32  platform::socket_write(OSSocket &sok, const void *buffer, u16 nBytesToSend)
{
	i32 ret = ::send(sok.socketID, (const char*)buffer, nBytesToSend, 0);
	if (ret == SOCKET_ERROR)
		return WSAGetLastError();
	
	assert(ret == nBytesToSend);
	return ret;
}



//*************************************************
eSocketError platform::socket_openAsUDP(OSSocket *sok)
{
	platform::socket_init(sok);

	//creo la socket
	sok->socketID = socket(AF_INET, SOCK_DGRAM, 0);
	if (sok->socketID == INVALID_SOCKET)
	{
		switch (WSAGetLastError())
		{
		case EACCES:            return eSocketError_denied;

		case EPROTONOSUPPORT:
		case EINVAL:
		case EAFNOSUPPORT:      return eSocketError_unsupported;

		case EMFILE:
		case ENFILE:            return eSocketError_tooMany;

		case ENOBUFS:
		case ENOMEM:            return eSocketError_noMem;

		default:                return eSocketError_unknown;
		}
	}

	//abilito delle opzioni di socket
	BOOL enable = TRUE;
	setsockopt(sok->socketID, SOL_SOCKET, SO_REUSEADDR, (const char*)&enable, sizeof(BOOL));
	//setsockopt(sok->socketID, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(BOOL));

	// non bloccante
	unsigned long lNonBlocking = 1;
	ioctlsocket(sok->socketID, FIONBIO, &lNonBlocking);


	return eSocketError_none;
}

//*************************************************
eSocketError platform::socket_UDPbind (OSSocket &sok, int portNumber)
{
	sockaddr_in		saAddress;
	saAddress.sin_family = AF_INET;
	saAddress.sin_port = htons(portNumber);
	saAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	if (SOCKET_ERROR == bind(sok.socketID, (LPSOCKADDR)&saAddress, sizeof(saAddress)))
		return eSocketError_unknown;

	return eSocketError_none;
}

//*************************************************
void test_test_test()
{
	INT iRetval;

	DWORD dwRetval;

	int i = 1;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	struct sockaddr_in  *sockaddr_ipv4;
	//    struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;

	char ipstringbuffer[46];
	DWORD ipbufferlength = 46;


	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	const char nodeName[] = {"127.0.0.1"};
	const char serviceName[] = { "80" };

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(nodeName, serviceName, &hints, &result);
	if (dwRetval != 0) 
	{
		printf("getaddrinfo failed with error: %d\n", dwRetval);
		return;
	}

	printf("getaddrinfo returned success\n");

	// Retrieve each address and print out the hex bytes
	for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			printf("\tIPv4 address %s\n",
				inet_ntoa(sockaddr_ipv4->sin_addr));
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

			// We use WSAAddressToString since it is supported on Windows XP and later
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
				ipstringbuffer, &ipbufferlength);
			if (iRetval)
				printf("WSAAddressToString failed with %u\n", WSAGetLastError());
			else
				printf("\tIPv6 address %s\n", ipstringbuffer);
			break;
		case AF_NETBIOS:
			printf("AF_NETBIOS (NetBIOS)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
		printf("\tSocket type: ");
		switch (ptr->ai_socktype) {
		case 0:
			printf("Unspecified\n");
			break;
		case SOCK_STREAM:
			printf("SOCK_STREAM (stream)\n");
			break;
		case SOCK_DGRAM:
			printf("SOCK_DGRAM (datagram) \n");
			break;
		case SOCK_RAW:
			printf("SOCK_RAW (raw) \n");
			break;
		case SOCK_RDM:
			printf("SOCK_RDM (reliable message datagram)\n");
			break;
		case SOCK_SEQPACKET:
			printf("SOCK_SEQPACKET (pseudo-stream packet)\n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_socktype);
			break;
		}
		printf("\tProtocol: ");
		switch (ptr->ai_protocol) {
		case 0:
			printf("Unspecified\n");
			break;
		case IPPROTO_TCP:
			printf("IPPROTO_TCP (TCP)\n");
			break;
		case IPPROTO_UDP:
			printf("IPPROTO_UDP (UDP) \n");
			break;
		default:
			printf("Other %ld\n", ptr->ai_protocol);
			break;
		}
		printf("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
		printf("\tCanonical name: %s\n", ptr->ai_canonname);
	}

	freeaddrinfo(result);

}

//*************************************************** 
void platform::socket_UDPSendBroadcast (OSSocket &sok, const u8 *buffer, u32 nBytesToSend, int porta, const char *subnetMask)
{
	//test_test_test();

	// Abilita il broadcast
	int i = 1;
	setsockopt(sok.socketID, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i));

	// Recupero info sul nome host, ip e porta di bind
	char hostName[128];
	char myIP[32];
	if (gethostname(hostName, sizeof(hostName)) != -1)
	{
		struct	hostent *clientHost;
		clientHost = gethostbyname(hostName);
		if (clientHost != 0)
		{
			struct	in_addr clientIP;
			memcpy(&clientIP, clientHost->h_addr_list[0], sizeof(struct in_addr));
			strcpy_s(myIP, 32, inet_ntoa(clientIP));
		}
	}

	// Broadcasta il messaggio
	//unsigned long	host_addr = inet_addr(myIP);		// local IP addr
	//unsigned long	host_addr = inet_addr("10.8.2.55");
	unsigned long	host_addr = inet_addr("10.8.0.0");
	//unsigned long	host_addr = inet_addr("192.168.231.1");
    unsigned long	net_mask = inet_addr(subnetMask);           // LAN netmask 255.255.255.0
	unsigned long	net_addr = host_addr & net_mask;				
	unsigned long	dir_bcast_addr = net_addr | (~net_mask);		

	sockaddr_in		saAddress;
	saAddress.sin_family = AF_INET;
	saAddress.sin_port = htons(porta);
	saAddress.sin_addr.s_addr = dir_bcast_addr;

	sendto (sok.socketID, (const char*)buffer, nBytesToSend, 0, (sockaddr*)&saAddress, sizeof(saAddress));

	// Disbilita il broadcast
	i = 0;
	setsockopt(sok.socketID, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i));
}


//*************************************************** 
u32 platform::socket_UDPSendTo (OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const OSNetAddr &addrTo)
{
	int ret = sendto (sok.socketID, (const char*)buffer, nBytesToSend, 0, rhea::netaddr::getSockAddr(addrTo), rhea::netaddr::getSockAddrLen(addrTo));
	if (SOCKET_ERROR == ret)
		return 0;
	return (u32)ret;
}

//*************************************************** 
u32 platform::socket_UDPReceiveFrom (OSSocket &sok, u8 *buffer, u32 nMaxBytesToRead, OSNetAddr *out_addrFrom)
{
	int	addrLen = rhea::netaddr::getSockAddrLen(*out_addrFrom);
	int ret = recvfrom(sok.socketID, (char*)buffer, nMaxBytesToRead, 0, rhea::netaddr::getSockAddr(*out_addrFrom), &addrLen);
	if (SOCKET_ERROR == ret)
		return 0;
	return (u32)ret;
}



#endif // WIN32
