#ifdef WIN32
#include "winOS.h"
#include <mbstring.h>
#include <shlobj.h>
#include "../../rhea.h"

struct	sWin32PlatformData
{
	HINSTANCE			hInst;
	u8					applicationPathNoSlash[256];
	u8					writableFolderPathNoSlash[256];
	u64					hiresTimerFreq;
	uint64_t			timeStarted;
	WSADATA				wsaData;
};

sWin32PlatformData	win32PlatformData;

//********************************************* 
bool platform::win32::utf8_towchar (const u8 *utf8_string, u32 nBytesToUse, wchar_t *out, u32 sizeInBytesOfOut)
{
	int n;
	if (nBytesToUse == u32MAX)
		n = -1;
	else
		n = (int)nBytesToUse;

	int result = MultiByteToWideChar (CP_UTF8, 0, (const char*)utf8_string, n, out, (sizeInBytesOfOut >> 1));
	if (0 == result)
	{
		DBGBREAK;
		return false;
	}

	if (out[result] != 0x00)
		out[result] = 0x00;

	return true;
}

//********************************************* 
bool platform::win32::wchar_to_utf8 (const wchar_t *wstring, u32 nBytesToUse, u8 *out, u32 sizeInBytesOfOut)
{
	int n;
	if (nBytesToUse == u32MAX)
		n = -1;
	else
		n = (int)nBytesToUse/2;

	int bytesWriten = WideCharToMultiByte  (CP_UTF8, 0, wstring, n, (char*)out, sizeInBytesOfOut, 0, 0);
	if (0 != bytesWriten)
	{
		if (n != -1)
			out[bytesWriten] = 0;
		return true;
	}

	DBGBREAK;
	return false;
}

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
	wchar_t wcString[MAX_PATH];
	GetModuleFileName (win32PlatformData.hInst, wcString, MAX_PATH);
	
	u8 temp_utf8[512];
	win32::wchar_to_utf8 (wcString, u32MAX, temp_utf8, sizeof(temp_utf8));
	rhea::fs::sanitizePathInPlace (temp_utf8);
	rhea::fs::extractFilePathWithOutSlash (temp_utf8, win32PlatformData.applicationPathNoSlash, sizeof(win32PlatformData.applicationPathNoSlash));


	//writable folder path
	SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wcString);
	{
		size_t	n = wcslen(wcString);
		for (size_t t = 0; t < n; t++)
		{
			if (wcString[t] == '\\')
				wcString[t] = '/';
		}

		win32::wchar_to_utf8 (wcString, u32MAX, win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash));
		rhea::fs::sanitizePathInPlace (win32PlatformData.writableFolderPathNoSlash); 
	}

	strcat_s((char*)win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash), "/");
	strcat_s((char*)win32PlatformData.writableFolderPathNoSlash, sizeof(win32PlatformData.writableFolderPathNoSlash), appName);
	rhea::fs::folderCreate(win32PlatformData.writableFolderPathNoSlash);


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
const u8* platform::getAppPathNoSlash()						{ return win32PlatformData.applicationPathNoSlash; }
const u8* platform::getPhysicalPathToWritableFolder()		{ return win32PlatformData.writableFolderPathNoSlash; }
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
