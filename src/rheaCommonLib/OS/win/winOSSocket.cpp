#ifdef WIN32
#include "OS/OS.h"
#include "winOSSocket.h"

#pragma comment(lib, "Ws2_32.lib")




//*************************************************
void platform::socket_init (OSSocket *sok)
{
	assert(sok != NULL);
	sok->socketID = INVALID_SOCKET;
	sok->readTimeoutMSec = 10000;
	sok->hEventNotify = INVALID_HANDLE_VALUE;
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
eSocketError platform::socket_openAsTCPServer (OSSocket *sok, int portNumber)
{
	socket_init(sok);

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
		//if (myerrno == WSAETIMEDOUT || myerrno == WSAEWOULDBLOCK) continue;

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
		case WSAEINVAL:
		case WSAECONNABORTED:
			return 0;

		case WSAEINPROGRESS:
		case WSAEWOULDBLOCK:
		case WSAETIMEDOUT:
			continue;
		}
	} while (OS_getTimeNowMSec() < timeToExitMSec);

	return -1;  //timeout
}

//*************************************************
i32  platform::socket_write(const OSSocket &sok, const void *buffer, u16 nBytesToSend)
{
	i32 ret = ::send(sok.socketID, (const char*)buffer, nBytesToSend, 0);
	if (ret != SOCKET_ERROR)
		return ret;
	
	return WSAGetLastError();
}
#endif // WIN32
