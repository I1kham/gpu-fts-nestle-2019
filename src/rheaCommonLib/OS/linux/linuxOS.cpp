#ifdef LINUX
#include "linuxOS.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ifaddrs.h>
#include "../../rhea.h"

struct	sLinuxPlatformData
{
    u8	*appPathNoSlash;
    u8	writableFolderPathNoSlash[256];
};
sLinuxPlatformData	linuxPlatformData;

//*******************************************************************
bool platform::internal_init(void *platformSpecificData UNUSED_PARAM, const char *appName)
{
	memset (&linuxPlatformData, 0, sizeof(linuxPlatformData));

    //usa la malloc per allocare il path
    linuxPlatformData.appPathNoSlash = (u8*)get_current_dir_name();
	rhea::fs::sanitizePathInPlace(linuxPlatformData.appPathNoSlash);
	
    sprintf_s((char*)linuxPlatformData.writableFolderPathNoSlash, sizeof(linuxPlatformData.writableFolderPathNoSlash), "%s/%s", linuxPlatformData.appPathNoSlash, appName);
	rhea::fs::sanitizePathInPlace(linuxPlatformData.writableFolderPathNoSlash);
	return true;
}

//*******************************************************************
void platform::internal_deinit()
{
	if (linuxPlatformData.appPathNoSlash)
		::free(linuxPlatformData.appPathNoSlash);
}

//**********************************************
const u8* platform::getAppPathNoSlash()                             { return linuxPlatformData.appPathNoSlash; }
const u8* platform::getPhysicalPathToWritableFolder()				{ return linuxPlatformData.writableFolderPathNoSlash; }
void* platform::alignedAlloc(size_t alignment, size_t size)			{ return aligned_alloc(alignment, size); }
void platform::alignedFree (void *p)								{ ::free(p); }

//**********************************************
uint64_t platform::getTimeNowMSec()
{
	static uint64_t timeStartedMSec = 0xFFFFFFFFFFFFFFFF;
	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC_RAW, &now);
	uint64_t timeNowMSec = now.tv_sec * 1000 + now.tv_nsec / 1000000;
	if (timeStartedMSec == 0xFFFFFFFFFFFFFFFF)
		timeStartedMSec = timeNowMSec;
	return (timeNowMSec - timeStartedMSec);
}


//*******************************************************************
void platform::sleepMSec (size_t msec)
{
    #define OSSLEEP_MSEC_TO_NANOSEC             1000000  // 1 millisec = 1.000.000 nanosec
    #define OSSLEEP_ONE_SECOND_IN_NANOSEC       1000000000

    //The value of the nanoseconds field must be in the range 0 to 999999999
    timespec sleepValue;
    sleepValue.tv_sec = 0;
    sleepValue.tv_nsec = msec * OSSLEEP_MSEC_TO_NANOSEC;
    while (sleepValue.tv_nsec >= OSSLEEP_ONE_SECOND_IN_NANOSEC)
    {
        sleepValue.tv_nsec -= OSSLEEP_ONE_SECOND_IN_NANOSEC;
        sleepValue.tv_sec++;
    }
    nanosleep(&sleepValue, NULL);
}


//*******************************************************************
void platform::getDateNow (u16 *out_year, u16 *out_month, u16 *out_day)
{
    time_t T = time(NULL);
    struct  tm tm = *localtime(&T);

    *out_year = tm.tm_year + 1900;
    *out_month = tm.tm_mon + 1;
    *out_day = tm.tm_mday;
}

//*******************************************************************
void platform::getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec)
{
    time_t T = time(NULL);
    struct  tm tm = *localtime(&T);

    *out_hour = tm.tm_hour;
    *out_min = tm.tm_min;
    *out_sec = tm.tm_sec;
}

//*******************************************************************
void platform::runShellCommandNoWait(const char *cmdIN)
{
    if (fork() != 0)
        return;

    char cmd[256];
    strcpy (cmd, cmdIN);

    const char* argv[16];
    memset (argv,0,sizeof(argv));

    u8 ct = 0;
    argv[ct++] = cmd;

    size_t n = strlen(cmd);
    size_t i=0;
    while (i<n)
    {
        if (cmd[i] == ' ')
        {
            while (cmd[i] == ' ' && i<n)
                cmd[i++] = 0x00;
            argv[ct++] = &(cmd[i]);
        }
        i++;
    }

    /*devo togliere il path da argv[0]
    n = strlen(argv[0]);
    while (n--)
    {
        if (argv[0][n]=='/')
        {
            argv[0] = &(argv[0][n+1]);
            break;
        }
    }*/

    //run
    execvp(cmd, (char* const*)argv);
}

//*******************************************************************
sNetworkAdapterInfo* platform::NET_getListOfAllNerworkAdpaterIPAndNetmask (rhea::Allocator *allocator, u32 *out_nRecordFound)
{
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

    *out_nRecordFound = 0;
    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;
        if (ifa->ifa_addr->sa_family == AF_INET)
            (*out_nRecordFound)++;
    }


    if (0 == (*out_nRecordFound))
        return NULL;

    u32 ct = 0;
	sNetworkAdapterInfo *ret = (sNetworkAdapterInfo*)RHEAALLOC(allocator, sizeof(sNetworkAdapterInfo) * (*out_nRecordFound));
	memset(ret, 0, sizeof(sNetworkAdapterInfo) * (*out_nRecordFound));

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (!ifa->ifa_addr)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            sprintf_s (ret[ct].name, sizeof(ret[ct].name), "%s", ifa->ifa_name);
            void *tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ret[ct].ip, INET_ADDRSTRLEN);

            tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_netmask)->sin_addr;
            inet_ntop(AF_INET, tmpAddrPtr, ret[ct].subnetMask, INET_ADDRSTRLEN);

            //printf("%s IP Address %s %s\n", ifa->ifa_name, ip, netmask);
            ct++;
        }
        /*else if (ifa->ifa_addr->sa_family == AF_INET6)
        { 	// check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);
        }
        */
    }
    if (ifAddrStruct!=NULL)
        freeifaddrs(ifAddrStruct);
    return ret;
}
#endif
