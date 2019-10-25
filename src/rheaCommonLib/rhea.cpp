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
	if (!internal_OSInit (platformSpecificInitData, rheaGlobals.appName))
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
    sysLogger = RHEANEW(memory_getDefaultAllocator(), Logger)(true);

    //aggiungo un file sysLogger
    {
        char logFileName[1024];
        sprintf_s (logFileName, sizeof(logFileName), "%s/log.txt", rhea::getPhysicalPathToAppFolder());

        rheaGlobals.fileLogger = RHEANEW(memory_getDefaultAllocator(), rhea::LogTargetFile)();
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
    RHEADELETE(memory_getDefaultAllocator(), sysLogger);
    RHEADELETE(memory_getDefaultAllocator(), rheaGlobals.fileLogger);
    rhea::internal_memory_deinit();

	internal_OSDeinit();
}

//***************************************************
bool rhea::isLittleEndian()										{ return rheaGlobals.bLittleEndiand; }
const char* rhea::getAppName()									{ return rheaGlobals.appName; }
const rhea::DateTime& rhea::getDateTimeAppStarted()				{ return rheaGlobals.dateTimeStarted; }
const rhea::Date& rhea::getDateAppStarted()						{ return rheaGlobals.dateTimeStarted.date; }
const rhea::Time24& rhea::getTimeAppStarted()					{ return rheaGlobals.dateTimeStarted.time; }
f32 rhea::random01()											{ return rheaGlobals.rnd.get01(); }
u32 rhea::randomU32(u32 iMax)									{ return rheaGlobals.rnd.getU32(iMax); }


