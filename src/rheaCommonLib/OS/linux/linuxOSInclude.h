#ifdef LINUX
#ifndef _linuxosinclude_h_
#define _linuxosinclude_h_
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include "../../rheaDataTypes.h"
#include "../../rheaEnumAndDefine.h"
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <termio.h>
#include <termios.h>
#include <unistd.h>


/***********************************************
 * c/c++ portability stuff
 */
#define sprintf_s snprintf
#define strcat_s(dest, size, source) strcat(dest,source)
#define strcpy_s(dest, size, source) strcpy(dest,source)
#define UNUSED_PARAM	__attribute__((unused))

 //definisce un tipo di dati valido per rappresentare un puntatore in un intero di qualche tipo
typedef uintptr_t IntPointer;


/***********************************************
 * debug helpers
 */
#ifdef _DEBUG
    #define	DBGBREAK	raise(SIGTRAP);
#else
    #define	DBGBREAK
#endif


/***********************************************
 * thread
 */
typedef pthread_t OSThread;
typedef void* (*OSThreadFunction)(void *userParam);



/***********************************************
 * serial port
 */
typedef struct sOSSerialPort
{
    int             fd;
    struct termios  config;
} OSSerialPort;


/***********************************************
 * OSEvent
 */
typedef struct sOSEvent
{
    epoll_event eventInfo;
    int         h;
    int         evfd;
} OSEvent;


/**************************************************************************
 * OSCriticalSection
 */
typedef struct sOSCriticalSection
{
    pthread_mutex_t cs;
} OSCriticalSection;


/***********************************************
 * socket
 */
typedef struct sOSSocket
{
    u32             readTimeoutMSec;
    int             socketID;
} OSSocket;

/***********************************************
 * filesystem
 */
typedef struct sOSFileFind
{
    DIR	*dirp;
    struct dirent *dp;
    char    strJolly[64];

                    sOSFileFind()					{ dirp = NULL; }
} OSFileFind;

#endif //_linuxosinclude_h_
#endif // LINUX
