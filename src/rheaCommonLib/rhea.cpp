#include <limits.h>
#include "rhea.h"
#include "rheaMemory.h"
#include "rheaThread.h"
#include "rheaLogTargetFile.h"
#include "rheaRandom.h"

namespace rhea
{
    Logger   *sysLogger = NULL;
}

struct sRheaGlobals
{
    rhea::LogTargetFile     *fileLogger;
	char					appName[64];
	bool					bLittleEndiand;
	rhea::DateTime			dateTimeStarted;
	rhea::Random			rnd;
};
sRheaGlobals	rheaGlobals;


//***************************************************
bool rhea::init (const char *appNameIN, void *platformSpecificInitData)
{
	//elimino caratteri strani da appName perchè questo nome mi serve per creare una cartella dedicata e quindi non voglio che contenga caratteri non validi
	if (NULL == appNameIN)
		sprintf_s(rheaGlobals.appName, sizeof(rheaGlobals.appName), "noname");
	else if (appNameIN[0] == 0x00)
		sprintf_s(rheaGlobals.appName, sizeof(rheaGlobals.appName), "noname");
	else
	{
		u8 i = 0;
		u8 t = 0;
		while (appNameIN[i])
		{
			char c = appNameIN[i++];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
				rheaGlobals.appName[t++] = c;
			else if (c == ' ')
				rheaGlobals.appName[t++] = '_';

			if (t == sizeof(rheaGlobals.appName) - 1)
				break;
		}
		rheaGlobals.appName[t] = 0x00;

		if (rheaGlobals.appName[0] == 0x00)
			sprintf_s(rheaGlobals.appName, sizeof(rheaGlobals.appName), "noname");
	}


	//init OS Stuff
	if (!platform::internal_init(platformSpecificInitData, rheaGlobals.appName))
		return false;

	//little/big endian ?
	u32 avalue = 0x01020304;
	const unsigned char *p = (const unsigned char*)&avalue;
	if (p[0] == 0x01)
		rheaGlobals.bLittleEndiand = false;
	else
		rheaGlobals.bLittleEndiand = true;
	
	//init memory
    rhea::internal_memory_init();

	//mi segno la data e l'ora di avvio
	rheaGlobals.dateTimeStarted.setNow();

    //init sysLogger
    sysLogger = RHEANEW(rhea::getSysHeapAllocator(), Logger)(true);

    //aggiungo un file sysLogger
    {
        char logFileName[1024];
        sprintf_s (logFileName, sizeof(logFileName), "%s/log.txt", rhea::getPhysicalPathToAppFolder());

        rheaGlobals.fileLogger = RHEANEW(rhea::getSysHeapAllocator(), rhea::LogTargetFile)();
        if (!rheaGlobals.fileLogger->init (logFileName, true))
            return false;
        sysLogger->addTarget(rheaGlobals.fileLogger);
    }

    sysLogger->log ("**********************************************************************************", false, false) << Logger::EOL;
    sysLogger->log ("",true,false) << Logger::EOL;
    sysLogger->log ("**********************************************************************************", false, false) << Logger::EOL;


    //init thread
    thread::internal_init();

	//generato random
	{
		u16 y, m, d;
		u8 hh, mm, ss;
		rhea::Date::getDateNow(&y, &m, &d);
		rhea::Time24::getTimeNow(&hh, &mm, &ss);
		u32 s = (y * 365 + m * 31 + d) * 24 * 3600 + hh * 3600 + mm * 60 + ss +(u32)getTimeNowMSec();
		rheaGlobals.rnd.seed(s);
	}

    //fine
    sysLogger->log ("rhea::init  DONE!") << Logger::EOL;
    return true;
}

//***************************************************
void rhea::deinit()
{
    sysLogger->log ("", false, false) << Logger::EOL;
    sysLogger->log ("Kernel::Deinit  STARTED") << Logger::EOL;


    sysLogger->log ("thread::Deinit  STARTED") << Logger::EOL;
    thread::internal_deinit();

    sysLogger->log ("mem::Deinit  STARTED") << Logger::EOL;
    RHEADELETE(rhea::getSysHeapAllocator(), sysLogger);
    RHEADELETE(rhea::getSysHeapAllocator(), rheaGlobals.fileLogger);
    rhea::internal_memory_deinit();

	platform::internal_deinit();
}

//***************************************************
bool rhea::isLittleEndian()										{ return rheaGlobals.bLittleEndiand; }
const char* rhea::getAppName()									{ return rheaGlobals.appName; }
const rhea::DateTime& rhea::getDateTimeAppStarted()				{ return rheaGlobals.dateTimeStarted; }
const rhea::Date& rhea::getDateAppStarted()						{ return rheaGlobals.dateTimeStarted.date; }
const rhea::Time24& rhea::getTimeAppStarted()					{ return rheaGlobals.dateTimeStarted.time; }
f32 rhea::random01()											{ return rheaGlobals.rnd.get01(); }
u32 rhea::randomU32(u32 iMax)									{ return rheaGlobals.rnd.getU32(iMax); }


//***************************************************
void rhea::netaddr::setFromSockAddr(OSNetAddr &me, const sockaddr_in &addrIN)			{ memcpy(&me.addr, &addrIN, sizeof(sockaddr_in)); }
void rhea::netaddr::setFromAddr(OSNetAddr &me, const OSNetAddr &addrIN)					{ memcpy(&me.addr, &addrIN.addr, sizeof(sockaddr_in)); }
void rhea::netaddr::setIPv4(OSNetAddr &me, const char *ip)								{ me.addr.sin_family = AF_INET; me.addr.sin_addr.s_addr = inet_addr(ip); }
void rhea::netaddr::setPort(OSNetAddr &me, int port)									{ me.addr.sin_family = AF_INET; me.addr.sin_port = htons(port); }
int rhea::netaddr::getPort (const OSNetAddr &me)										{ return ntohs(me.addr.sin_port); }
sockaddr* rhea::netaddr::getSockAddr(const OSNetAddr &me)								{ return (sockaddr*)&me.addr; }
int rhea::netaddr::getSockAddrLen(const OSNetAddr &me)									{ return sizeof(me.addr); }

void rhea::netaddr::getIPv4 (const OSNetAddr &me, char *out)
{
	//todo
	out[0] = 0x00;
	const char *ip = inet_ntoa(me.addr.sin_addr);
	if (NULL != ip && ip[0] != 0x00)
		strncpy(out, ip, 16);
}

bool rhea::netaddr::compare(const OSNetAddr &a, const OSNetAddr &b)
{
	if (rhea::netaddr::getPort(a) != netaddr::getPort(b))
		return false;
	char ipA[32], ipB[32];
	netaddr::getIPv4(a, ipA);
	netaddr::getIPv4(b, ipB);
	if (strcasecmp(ipA, ipB) != 0)
		return false;
	return true;
}


//*************************************************** 
void rhea::socket::UDPSendBroadcast(OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const char *ip, int portNumber, const char *subnetMask)
{
	// Abilita il broadcast
	int i = 1;
	setsockopt(sok.socketID, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i));

	// Broadcasta il messaggio
	const unsigned long	host_addr = inet_addr(ip);
	const unsigned long	net_mask = inet_addr(subnetMask);
	const unsigned long	net_addr = host_addr & net_mask;
	const unsigned long	dir_bcast_addr = net_addr | (~net_mask);

	sockaddr_in		saAddress;
	saAddress.sin_family = AF_INET;
	saAddress.sin_port = htons(portNumber);
	saAddress.sin_addr.s_addr = dir_bcast_addr;
	sendto(sok.socketID, (const char*)buffer, nBytesToSend, 0, (sockaddr*)&saAddress, sizeof(saAddress));


	// Disabilita il broadcast
	i = 0;
	setsockopt (sok.socketID, SOL_SOCKET, SO_BROADCAST, (char*)&i, sizeof(i));
}