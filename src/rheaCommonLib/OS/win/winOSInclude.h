#ifdef WIN32
#ifndef _winOSInclude_h_
#define _winOSInclude_h_

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <assert.h>
#include <Winsock2.h>
#include <stdio.h>
#include "../rheacommonlib/rheaDataTypes.h"
#include "../rheacommonlib/rheaEnumAndDefine.h"



/***********************************************
 * c/c++ portability stuff
 */
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strtok_r strtok_s

#define UNUSED_PARAM

//definisce un tipo di dati valido per rappresentare un puntatore in un intero di qualche tipo
#define IntPointer uintptr_t


/***********************************************
 * debug helpers
 */
#ifdef _DEBUG
	#define	DBGBREAK	_asm int 3;
#else
	#define	DBGBREAK
#endif


/***********************************************
 * thread
 */
typedef HANDLE OSThread;
typedef void* (*OSThreadFunction)(void *userParam);



 /***********************************************
  * serial port
  */
typedef struct sOSSerialPort
{
	int todo;
} OSSerialPort;


/***********************************************
 * OSEvent
 */
typedef struct sOSEvent
{
	HANDLE	h;
} OSEvent;


/**************************************************************************
 * OSCriticalSection
 */
typedef struct sOSCriticalSection
{
	CRITICAL_SECTION cs;
} OSCriticalSection;


/***********************************************
 * socket
 */
typedef struct sOSSocket
{
	u32             readTimeoutMSec;
	SOCKET          socketID;
	HANDLE			hEventNotify;
} OSSocket;





#endif //_winOSInclude_h_
#endif //WIN32
