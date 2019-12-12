#ifdef LINUX
#include "linuxOS.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "../../rhea.h"

struct	sLinuxPlatformData
{
	char	*appPathNoSlash;
	char	writableFolderPathNoSlash[256];
};
sLinuxPlatformData	linuxPlatformData;

//*******************************************************************
bool platform::internal_init(void *platformSpecificData UNUSED_PARAM, const char *appName)
{
	memset (&linuxPlatformData, 0, sizeof(linuxPlatformData));

	//usa la malloc per allocare il path
	linuxPlatformData.appPathNoSlash = get_current_dir_name();
	rhea::fs::sanitizePathInPlace(linuxPlatformData.appPathNoSlash);
	
	sprintf_s(linuxPlatformData.writableFolderPathNoSlash, sizeof(linuxPlatformData.writableFolderPathNoSlash), "%s/%s", linuxPlatformData.appPathNoSlash, appName);
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
const char* platform::getAppPathNoSlash()							{ return linuxPlatformData.appPathNoSlash; }
const char* platform::getPhysicalPathToWritableFolder()				{ return linuxPlatformData.writableFolderPathNoSlash; }
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


#endif
