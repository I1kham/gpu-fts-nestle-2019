#ifdef WIN32
#include "winOS.h"
#include <mbstring.h>
#include <shlobj.h>

struct	sWin32PlatformData
{
	HINSTANCE			hInst;
	char				applicationPathNoSlash[256];
	char				writableFolderPathNoSlash[256];
	u64					hiresTimerFreq;
	uint64_t			timeStarted;
	WSADATA				wsaData;
};

sWin32PlatformData	win32PlatformData;

//**********************************************
bool platform::internal_init (void *platformSpecificData, const char *appName)
{
	memset(&win32PlatformData, 0, sizeof(win32PlatformData));

	win32PlatformData.hInst = *((HINSTANCE*)platformSpecificData);

	//timer
	QueryPerformanceFrequency((LARGE_INTEGER*)&win32PlatformData.hiresTimerFreq);
	//win32PlatformData.hiresTimerFreq /= 1000000; //per avere il time in microsec
	win32PlatformData.hiresTimerFreq   /= 1000; //per avere il time in msec
	
	QueryPerformanceCounter((LARGE_INTEGER*)&win32PlatformData.timeStarted);

	//application path
	GetModuleFileName(win32PlatformData.hInst, win32PlatformData.applicationPathNoSlash, sizeof(win32PlatformData.applicationPathNoSlash));
	size_t	n = _mbstrlen(win32PlatformData.applicationPathNoSlash) - 1;
	for (size_t t = 0; t < n; t++)
	{
		if (win32PlatformData.applicationPathNoSlash[t] == '\\')
			win32PlatformData.applicationPathNoSlash[t] = '/';
	}
	while (n && win32PlatformData.applicationPathNoSlash[n] != '/')
		win32PlatformData.applicationPathNoSlash[n--] = 0x00;
	win32PlatformData.applicationPathNoSlash[n--] = 0x00;


	//writable folder path
	memset (win32PlatformData.writableFolderPathNoSlash, 0, sizeof(win32PlatformData.writableFolderPathNoSlash));
	SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, win32PlatformData.writableFolderPathNoSlash);
	_mbscat_s((unsigned char*)win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash), (unsigned char*)"/");
	n = _mbstrlen(win32PlatformData.writableFolderPathNoSlash) - 1;
	for (size_t t = 0; t < n; t++)
	{
		if (win32PlatformData.writableFolderPathNoSlash[t] == '\\')
			win32PlatformData.writableFolderPathNoSlash[t] = '/';
	}
	win32PlatformData.writableFolderPathNoSlash[_mbstrlen(win32PlatformData.writableFolderPathNoSlash) - 1] = 0;
	strcat_s(win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash), "/");
	strcat_s(win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash), appName);
	CreateDirectory(win32PlatformData.writableFolderPathNoSlash, NULL);


	//initialize Winsock
	if (0 != WSAStartup(MAKEWORD(2, 2), &win32PlatformData.wsaData))
		return false;

	return true;
}

//**********************************************
void platform::internal_deinit()
{
	WSACleanup();
}

//**********************************************
void* platform::alignedAlloc(size_t alignment, size_t size)
{
	void *p = _aligned_malloc(size, alignment);
	assert(NULL != p);
	return p;
}

//**********************************************
void platform::alignedFree(void *p)							{ _aligned_free(p);  }
const char* platform::getAppPathNoSlash()					{ return win32PlatformData.applicationPathNoSlash; }
const char* platform::getPhysicalPathToWritableFolder()		{ return win32PlatformData.writableFolderPathNoSlash; }
void platform::sleepMSec(size_t msec)						{ ::Sleep(msec); }

//**********************************************
uint64_t platform::getTimeNowMSec()
{
	uint64_t	timeNow;
	QueryPerformanceCounter((LARGE_INTEGER*)&timeNow);
	timeNow -= win32PlatformData.timeStarted;
	timeNow /= win32PlatformData.hiresTimerFreq; //time in msec
	return timeNow;
}

//*******************************************************************


//*******************************************************************
void platform::getDateNow(u16 *out_year, u16 *out_month, u16 *out_day)
{
	SYSTEMTIME s;
	GetLocalTime(&s);

	(*out_year) = (u16)s.wYear;
	(*out_month) = (u16)s.wMonth;
	(*out_day) = (u16)s.wDay;
}

//*******************************************************************
void platform::getTimeNow(u8 *out_hour, u8 *out_min, u8 *out_sec)
{
	SYSTEMTIME s;
	GetLocalTime(&s);

	(*out_hour) = (u8)s.wHour;
	(*out_min) = (u8)s.wMinute;
	(*out_sec) = (u8)s.wSecond;
}


//*******************************************************************
void platform::runShellCommandNoWait (const char *cmdIN)
{
	//non implementata
	DBGBREAK;
}

#endif //WIN32
