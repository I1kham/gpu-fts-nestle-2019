#include "ESAPICore.h"
#include "ESAPI.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "ESAPIModuleRaw.h"
#include "ModuleRasPI.h"

using namespace esapi;

//*********************************************************
Core::Core()
{
	glob.localAllocator = NULL;
	glob.logger = &nullLogger;
	rhea::rs232::setInvalid (glob.com);
    glob.moduleInfo.type = esapi::eExternalModuleType_unknown;
    glob.moduleInfo.verMajor = glob.moduleInfo.verMinor = 0;

    curModule = NULL;
}

//*********************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        glob.logger = &nullLogger;
    else
        glob.logger = loggerIN;
}

//*********************************************************
bool Core::open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW)
{
	const bool SERIAL_IS_BLOCKING = false;

    glob.hCPUServiceChannelW = hCPUServiceChannelW;

    glob.logger->log ("esapi::Core::open\n");
    glob.logger->incIndent();

	glob.logger->log ("com=%s   ", serialPort);
    if (!rhea::rs232::open(&glob.com, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        glob.logger->log ("FAILED. unable to open port [%s]\n", serialPort);
        glob.logger->decIndent();
        return false;
    }
	glob.logger->log ("OK\n");

    //creo la service msg Q sulla quale ricevo msg da thread esterni
    rhea::thread::createMsgQ (&glob.serviceMsgQR, &glob.serviceMsgQW);

    glob.localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");

    //subscriber list
    glob.subscriberList.setup (glob.localAllocator);

    //spawn del modulo raw
    esapi::ModuleRaw *m = RHEANEW(glob.localAllocator, esapi::ModuleRaw)();
    if (!m->setup (&glob))
    {
        RHEADELETE(glob.localAllocator, m);
	    glob.logger->log ("FAIL\n");
	    glob.logger->decIndent();
    	glob.logger->decIndent();
        return false;
    }
    curModule = m;
	glob.logger->log ("OK\n");
	glob.logger->decIndent();
    return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (glob.com);

    if (curModule)
    {
        RHEADELETE(glob.localAllocator, curModule);
        curModule = NULL;
    }

    if (glob.localAllocator)
    {
        rhea::thread::deleteMsgQ (glob.serviceMsgQR, glob.serviceMsgQW);

        glob.subscriberList.unsetup();
        RHEADELETE(rhea::getSysHeapAllocator(), glob.localAllocator);
        glob.localAllocator = NULL;
    }
}

//*********************************************************
void Core::run()
{
    //run del module raw
    eExternalModuleType ret = curModule->run();
    RHEADELETE(glob.localAllocator, curModule);
    curModule = NULL;
    glob.logger->log ("Core::run() => module raw deleted\n");

    //se il module raw ritorna un particolare [eExternalModuleType], allora devo spawnarlo a mandarlo in run, altrimenti
    //vuol dire che abbiamo finito
    switch (ret)
    {
    default:
        break;

    case esapi::eExternalModuleType_rasPI_wifi_REST:
        {
            glob.logger->log ("Core::run() => spawn moduleRasPI\n");
            ModuleRasPI *m = RHEANEW(glob.localAllocator, ModuleRasPI)();
            m->run();
            RHEADELETE(m, curModule);
        }
        break;
    }
    glob.logger->log ("Core::run() => closing...\n");
    priv_close();
}

