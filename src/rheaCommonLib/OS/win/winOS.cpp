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

//*******************************************************************
#include "Iphlpapi.h"
#include "ws2tcpip.h"
#pragma comment(lib, "IPHLPAPI.lib")
sNetworkAdapterInfo* platform::NET_getListOfAllNerworkAdpaterIPAndNetmask (rhea::Allocator *allocator, u32 *out_nRecordFound)
{
	*out_nRecordFound = 0;

	rhea::Allocator *tempAllocator = rhea::getScrapAllocator();

	// Before calling AddIPAddress we use GetIpAddrTable to get an adapter to which we can add the IP.
	DWORD dwTableSize = sizeof(MIB_IPADDRTABLE);
	PMIB_IPADDRTABLE pIPAddrTable = (MIB_IPADDRTABLE *)RHEAALLOC(tempAllocator,dwTableSize);
	if (GetIpAddrTable(pIPAddrTable, &dwTableSize, 0) == ERROR_INSUFFICIENT_BUFFER)
	{
		RHEAFREE(tempAllocator, pIPAddrTable);
		pIPAddrTable = (MIB_IPADDRTABLE *)RHEAALLOC(tempAllocator, dwTableSize);
	}

	// Make a second call to GetIpAddrTable to get the actual data we want
	DWORD err = GetIpAddrTable(pIPAddrTable, &dwTableSize, 0);
	if (err != NO_ERROR) 
	{
		/*
		LPVOID lpMsgBuf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL))
		{
			printf("\tError: %s", (const char*)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		*/
		return NULL;
	}

	if (pIPAddrTable->dwNumEntries == 0)
		return NULL;
	if (pIPAddrTable->dwNumEntries > 250)
		*out_nRecordFound = 250;
	else
		*out_nRecordFound = (u8)pIPAddrTable->dwNumEntries;
	
	sNetworkAdapterInfo *ret = (sNetworkAdapterInfo*)RHEAALLOC(allocator, sizeof(sNetworkAdapterInfo) * (*out_nRecordFound));
	memset(ret, 0, sizeof(sNetworkAdapterInfo) * (*out_nRecordFound));
	for (u8 i = 0; i < (*out_nRecordFound); i++)
	{
		IN_ADDR IPAddr;

		//printf("\n\tInterface Index[%d]:\t%ld\n", i, pIPAddrTable->table[i].dwIndex);
		sprintf_s (ret->name, sizeof(ret->name), "ip%d", i);
		
		IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwAddr;
		InetNtopA(AF_INET, &IPAddr, ret[i].ip, sizeof(ret[i].ip));
		//printf("\tIP Address[%d]:     \t%s\n", i, ret[i].ip);
		
		IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwMask;
		InetNtopA(AF_INET, &IPAddr, ret[i].subnetMask, sizeof(ret[i].subnetMask));
		//printf("\tSubnet Mask[%d]:    \t%s\n", i, ret[i].subnetMask);
		
		/*IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwBCastAddr;
		printf("\tBroadCast[%d]:      \t%s (%ld)\n", i, InetNtop(AF_INET, &IPAddr, sAddr, sizeof(sAddr)), pIPAddrTable->table[i].dwBCastAddr);
		
		printf("\tReassembly size[%d]:\t%ld\n", i, pIPAddrTable->table[i].dwReasmSize);
		
		printf("\tType and State[%d]:", i);
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_PRIMARY)
			printf("\tPrimary IP Address");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DYNAMIC)
			printf("\tDynamic IP Address");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)
			printf("\tAddress is on disconnected interface");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DELETED)
			printf("\tAddress is being deleted");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_TRANSIENT)
			printf("\tTransient address");*/
		//printf("\n");
	}

	if (pIPAddrTable) 
		RHEAFREE(tempAllocator, pIPAddrTable);

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

	rhea::Allocator *allocator = rhea::getScrapAllocator();

	PIP_ADAPTER_INFO AdapterInfo;
	AdapterInfo = (IP_ADAPTER_INFO *)RHEAALLOC(allocator, sizeof(IP_ADAPTER_INFO));
	if (AdapterInfo == NULL)
	{
		out_macAddress[0] = 0x00;
		DBGBREAK;
		return false;
	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);
	if (ERROR_BUFFER_OVERFLOW == GetAdaptersInfo(AdapterInfo, &dwBufLen))
	{
		RHEAFREE(allocator, AdapterInfo);
		AdapterInfo = (IP_ADAPTER_INFO *)RHEAALLOC(allocator, dwBufLen);
		if (AdapterInfo == NULL)
		{
			out_macAddress[0] = 0x00;
			DBGBREAK;
			return false;
		}
	}

	if (GetAdaptersInfo(AdapterInfo, &dwBufLen) == NO_ERROR)
	{
		// Contains pointer to current adapter info
		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
		while (pAdapterInfo)
		{
			if (strcmp(pAdapterInfo->IpAddressList.IpAddress.String, "0.0.0.0") != 0)
			{
				// technically should look at pAdapterInfo->AddressLength and not assume it is 6.
				//sprintf(mac_addr, "%02X:%02X:%02X:%02X:%02X:%02X", pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
				//printf("Address: %s, mac: %s\n", pAdapterInfo->IpAddressList.IpAddress.String, mac_addr);
				//printf("\n");

				if (pAdapterInfo->AddressLength == 6)
					sprintf_s(out_macAddress, sizeOfMacAddress, "%02X%02X%02X%02X%02X%02X", pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], pAdapterInfo->Address[3], pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
				else
					sprintf_s(out_macAddress, sizeOfMacAddress, "%02X%02X%02X%02X", pAdapterInfo->Address[0], pAdapterInfo->Address[1], pAdapterInfo->Address[2], pAdapterInfo->Address[3]);

				RHEAFREE(allocator, AdapterInfo);
				return true;
			}

			pAdapterInfo = pAdapterInfo->Next;
		}
	}
	RHEAFREE(allocator, AdapterInfo);
	return false;
}
#endif //WIN32
