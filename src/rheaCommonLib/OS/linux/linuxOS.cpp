#ifdef LINUX
#include "linuxOS.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <ifaddrs.h>
#include <linux/if.h>
#include <sys/reboot.h>
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
const u8 *platform::getAppPathNoSlash()                             { return linuxPlatformData.appPathNoSlash; }
const u8 *platform::getPhysicalPathToWritableFolder()				{ return linuxPlatformData.writableFolderPathNoSlash; }
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
void platform::reboot()
{
    ::sync();
    ::setuid (0);
    ::reboot(RB_AUTOBOOT);
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
bool platform::runShellCommandNoWait (const u8 *fullPathExeName, const u8 *cmdLineParameters, const u8 *workingDir UNUSED_PARAM)
{
	//from: http://www.qnx.com/developers/docs/qnxcar2/index.jsp?topic=%2Fcom.qnx.doc.neutrino.lib_ref%2Ftopic%2Fe%2Fexecvpe.html   
	//The execvpe() function uses the paths listed in the PATH environment variable to locate the program to be loaded, provided that the following conditions are met:
	//
	//The file argument identifies the name of program to be loaded.
	//If no path character (/) is included in the name, an attempt is made to load the program from one of the paths in the PATH environment variable.
	//If PATH isn't defined, the current working directory is used.
	//If a path character (/) is included in the name, the program is loaded from the path specified in file.
	//The process is started with the argument specified in argv, a NULL-terminated array of NULL-terminated strings. The argv[0] entry should point to a filename associated with the program being loaded. The argv argument can't be NULL but argv[0] can be NULL if no arguments are required.
	//
	//The new process's environment is specified in envp, a NULL-terminated array of NULL-terminated strings. envp cannot be NULL, but envp[0] can be a NULL pointer if no environment strings are passed.
    if (fork() != 0)
        return false;

    const char* argv[16];
    memset (argv,0,sizeof(argv));

    u8 ct = 0;
    argv[ct++] = reinterpret_cast<const char*>(fullPathExeName);

    u32 n = rhea::string::utf8::lengthInBytes(cmdLineParameters);
    u8 *cmd = RHEAALLOCT(u8*, rhea::getScrapAllocator(), n+2);
    memcpy (cmd, cmdLineParameters, n);
    cmd[n] = 0x00;

    if (cmd[0] != 0x00)
    {
        argv[ct++] = reinterpret_cast<const char*>(cmd);

        u32 i=0;
        while (i < n)
        {
            if (cmd[i] == ' ')
            {
                while (cmd[i] == ' ' && i<n)
                    cmd[i++] = 0x00;
                argv[ct++] = reinterpret_cast<const char*>(&(cmd[i]));
            }
            i++;
        }
    }

    /*for (u8 i=0; i<10; i++)
    {
        if (argv[i] != 0x00)
            printf ("======arg %d: %s\n", i, argv[i]);
    }
    */


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

/*
    char* envp[16];
    memset (envp,0,sizeof(envp));

    i = 0;
    envp[i] = getenv("PATH");
    if (NULL != envp[i])
    {
        const u32 len = 6 + strlen(envp[i]);
        char *s = RHEAALLOCT(char*, rhea::getScrapAllocator(), len);
        sprintf_s (s, len, "PATH=%s", envp[i]);
        envp[i] = s;
        i++;
    }
*/
    //run
    //execvpe (reinterpret_cast<const char*>(fullPathExeName), (char* const*)argv, (char* const*)envp);
    execvp (reinterpret_cast<const char*>(fullPathExeName), (char* const*)argv);

    RHEAFREE(rhea::getScrapAllocator(), cmd);

/*    i = 0;
    while (envp[i] != NULL)
    {
        RHEAFREE(rhea::getScrapAllocator(), envp[i]);
        i++;
    }
    */
    return true;
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

//*******************************************************************
bool platform::NET_getMACAddress (char *out_macAddress, u32 sizeOfMacAddress)
{
	if (sizeOfMacAddress < 16)
	{
		out_macAddress[0] = 0x00;
		DBGBREAK;
		return false;
	}
	out_macAddress[0] = 0x00;

	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) 
	{
        sprintf_s (out_macAddress, sizeOfMacAddress, "%02X%02X%02X%02X%02X%02X",
                    (unsigned char)s.ifr_addr.sa_data[0],
                    (unsigned char)s.ifr_addr.sa_data[1],
                    (unsigned char)s.ifr_addr.sa_data[2],
                    (unsigned char)s.ifr_addr.sa_data[3],
                    (unsigned char)s.ifr_addr.sa_data[4],
                    (unsigned char)s.ifr_addr.sa_data[5]);
		return true;
	}
	return false;
}

//*******************************************************************
bool platform::BROWSER_open (const u8 *url, bool bFullscreen)
{
    u8 s[512];
    if (bFullscreen)
       rhea::string::utf8::spf (s, sizeof(s), "--no-sandbox --test-type --remote-debugging-port=9222 --disable-pinch --disable-session-crashed-bubble --overscroll-history-navigation=0 --incognito --kiosk %s", url);
    else
        rhea::string::utf8::spf (s, sizeof(s), "--no-sandbox --test-type %s", url);
    platform::runShellCommandNoWait (reinterpret_cast<const u8*>("chromium"), s, NULL);
    return true;
}

//*******************************************************************
void platform::BROWSER_closeAllInstances ()
{
    //platform::runShellCommandNoWait ("/usr/bin/pkill --oldest --signal TERM -f chromium");

    FILE *fp = popen ("/usr/bin/pkill --signal TERM chromium", "r");
    if (NULL == fp)
        return;
    char result[256];
    fgets (result, sizeof(result), fp);
    pclose (fp);
}

//*****************************************************
bool platform::executeShellCommandAndStoreResult (const char *shellCommand, char *out_result, u32 sizeOfOutResult)
{
    FILE *fp = popen (shellCommand, "r");
    if (NULL == fp)
        return false;
    fgets (out_result, sizeOfOutResult, fp);
    fclose (fp);
    return true;
}
#endif
