#include <process.h>
#include "../CPUBridge/CPUBridge.h"
#include "../CPUBridge/CPUChannelFakeCPU.h"
#include "../SocketBridge/SocketBridge.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../CPUBridge/EVADTSParser.h"
#include "TaskCopyFolderToFolder.h"
#include "TaskExportGUIToUserFolder.h"
#include "TaskDeleteFolder.h"
#include "TaskImportExistingGUI.h"
#include "TaskExportMobileTPGUIToUserFolder.h"
#include "varie.h"

#define		RHEAMEDIA2_VERSIONE L"2.5"
#define		RHEAMEDIA2_DATA		L"2021/07/26"


wchar_t chromeFullPathAndName[256];
wchar_t chromeHome[256];

//*****************************************************
void starChrome()
{
	_wspawnl(_P_NOWAIT, chromeFullPathAndName, L"chrome.exe", chromeHome, NULL);
}

//*****************************************************
bool startCPUBridge()
{
#ifdef _DEBUG
	rhea::StdoutLogger loggerSTD;
	//rhea::NullLogger loggerSTD;
	rhea::ISimpleLogger *logger = &loggerSTD;
#else
	rhea::NullLogger loggerNULL;
	rhea::ISimpleLogger *logger = &loggerNULL;
#endif


	//apro un canale di comunicazione con una finta CPU
	cpubridge::CPUChannelFakeCPU *chToCPU = new cpubridge::CPUChannelFakeCPU(); 
	if (!chToCPU->open(logger))
		return false;
	

	//creo il thread di CPUBridge
	rhea::HThread hCPUThread;
	HThreadMsgW hCPUServiceChannelW;

	if (!cpubridge::startServer(chToCPU, logger, &hCPUThread, &hCPUServiceChannelW))
		return false;

	//starto socketBridge che a sua volta si iscriverà a CPUBridge
	rhea::HThread hSocketBridgeThread;
	if (!socketbridge::startServer(logger, hCPUServiceChannelW, true, true, &hSocketBridgeThread))
	{
		printf("ERROR: can't open socket\n.");
		return false;
	}

	//Aggiungo i task
	socketbridge::addTask<TaskCopyFolderToFolder>(hSocketBridgeThread, "copyFolderToFolder");
	socketbridge::addTask<TaskExportGUIToUserFolder>(hSocketBridgeThread, "exportGUIToUserFolder");
	socketbridge::addTask<TaskDeleteFolder>(hSocketBridgeThread, "deleteFolder");
	socketbridge::addTask<TaskImportExistingGUI>(hSocketBridgeThread, "importExistingGUI");
	socketbridge::addTask<TaskExportMobileTPGUIToUserFolder>(hSocketBridgeThread, "exportMobileTPGUIToUserFolder");

	//apro chrome
	printf("opening chrome\n");
	rhea::thread::sleepMSec(100);
	starChrome();
	::ShowWindow(::GetConsoleWindow(), SW_MINIMIZE);


	//attendo che il thread CPU termini
	rhea::thread::waitEnd(hCPUThread);

	return true;
}

//*****************************************************
bool findChrome (HKEY rootKey, wchar_t *out, u32 sizeofOut)
{
	#define CHROME_IN_REGISTRY L"Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe"

	out[0] = 0;

	HKEY hKey;
	if (ERROR_SUCCESS != RegOpenKeyEx(rootKey, CHROME_IN_REGISTRY, 0, KEY_QUERY_VALUE, &hKey))
		return false;

	DWORD type = REG_MULTI_SZ;
	DWORD sizeOfOut = sizeofOut;
	if (ERROR_SUCCESS != RegQueryValueEx(hKey, NULL, NULL, &type, (LPBYTE)out, &sizeOfOut))
		return false;

	if (out[0] != 0x00)
		return true;
	return false;
}

//*****************************************************
bool findChrome()
{
	//Cerco chrome
	if (!findChrome(HKEY_CURRENT_USER, chromeFullPathAndName, sizeof(chromeFullPathAndName)))
	{
		if (!findChrome(HKEY_LOCAL_MACHINE, chromeFullPathAndName, sizeof(chromeFullPathAndName)))
		{
			MessageBox(NULL, L"Unable to find the Chrome browser.\nPlease install chrome browser and try again.", L"rheaMedia2", MB_OK);
			return false;
		}
	}

	//determino il path della home
	wchar_t pathToAppFolder[512];
	platform::win32::utf8_towchar (rhea::getPhysicalPathToAppFolder(), u32MAX, pathToAppFolder, sizeof(pathToAppFolder));
	swprintf_s(chromeHome, _countof(chromeHome), L"\"file:///%s/home/startup.html\"", pathToAppFolder);

	return true;
}

//*****************************************************
bool isSMUAlreadyRunning()
{
	OSSocket sokUDP;
	rhea::socket::init(&sokUDP);
	rhea::socket::openAsUDP(&sokUDP);

	u8 buffer[32];
	u8 ct = 0;
	buffer[ct++] = 'R';
	buffer[ct++] = 'H';
	buffer[ct++] = 'E';
	buffer[ct++] = 'A';
	buffer[ct++] = 'H';
	buffer[ct++] = 'E';
	buffer[ct++] = 'l';
	buffer[ct++] = 'l';
	buffer[ct++] = 'O';

	OSNetAddr addr;
	rhea::netaddr::setIPv4 (addr, "127.0.0.1");
	rhea::netaddr::setPort(addr, 2281);
	rhea::socket::UDPSendTo(sokUDP, buffer, ct, addr);
	u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	printf("Checking if rheaMedia2 is already running");
	while (rhea::getTimeNowMSec() < timeToExitMSec)
	{
		OSNetAddr from;
		u8 buffer[32];

		rhea::thread::sleepMSec(200);
		printf(".");

		u32 nBytesRead = rhea::socket::UDPReceiveFrom(sokUDP, buffer, sizeof(buffer), &from);
		if (nBytesRead == 9)
		{
			printf("\n");
			if (memcmp(buffer, "rheaHelLO", 9) == 0)
				return true;
		}
	}

	printf("\n");
	return false;
}

//*****************************************************
int main()
{
	HINSTANCE hInst = NULL;
	rhea::init("rheaMedia20", &hInst);
	

	//varie_patchAllTemplate_updateLanguageNames();



	wchar_t nameAndVersion[128];
	swprintf_s(nameAndVersion, _countof(nameAndVersion), L"RheaMedia2 - Version " RHEAMEDIA2_VERSIONE " - " RHEAMEDIA2_DATA);

	SetConsoleTitle(nameAndVersion);
	wprintf(nameAndVersion);
	printf("\n\n");
	if (findChrome())
	{
		if (isSMUAlreadyRunning())
			starChrome();
		else
			startCPUBridge();
	}


	rhea::deinit();
	return 0;
}
