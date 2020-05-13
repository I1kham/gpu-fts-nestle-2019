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
#include "varie.h"

#define		RHEAMEDIA2_VERSIONE "2.0.2"


char chromeFullPathAndName[256];
char chromeHome[256];

//*****************************************************
void starChrome()
{
	_spawnl(_P_NOWAIT, chromeFullPathAndName, "chrome.exe", chromeHome, NULL);
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
	if (!socketbridge::startServer(logger, hCPUServiceChannelW, true , &hSocketBridgeThread))
	{
		printf("ERROR: can't open socket\n.");
		return false;
	}

	//Aggiungo i task
	socketbridge::addTask<TaskCopyFolderToFolder>(hSocketBridgeThread, "copyFolderToFolder");
	socketbridge::addTask<TaskExportGUIToUserFolder>(hSocketBridgeThread, "exportGUIToUserFolder");
	socketbridge::addTask<TaskDeleteFolder>(hSocketBridgeThread, "deleteFolder");
	socketbridge::addTask<TaskImportExistingGUI>(hSocketBridgeThread, "importExistingGUI");

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
bool findChrome (HKEY rootKey, char *out, u32 sizeofOut)
{
	#define CHROME_IN_REGISTRY "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\chrome.exe"

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
			MessageBox(NULL, "Unable to find the Chrome browser.\nPlease install chrome browser and try again.", "rheaMedia2.0", MB_OK);
			return false;
		}
	}

	//determino il path della home
	sprintf_s(chromeHome, sizeof(chromeHome), "file:///%s/home/startup.html", rhea::getPhysicalPathToAppFolder());

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



	char nameAndVersion[128];
	sprintf_s(nameAndVersion, sizeof(nameAndVersion), "RheaMedia2 - Version " RHEAMEDIA2_VERSIONE " - 2020/04/10");

	SetConsoleTitle(nameAndVersion);
	printf(nameAndVersion);
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
