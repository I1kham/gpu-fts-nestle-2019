#include "ESAPICore.h"
#include "ESAPI.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "ESAPIModuleRaw.h"
#include "ESAPIModuleRasPI.h"

using namespace esapi;

//*********************************************************
Core::Core()
{
	shared.localAllocator = NULL;
	shared.logger = &nullLogger;
	rhea::rs232::setInvalid (shared.com);
    shared.moduleInfo.type = esapi::eExternalModuleType_none;
    shared.moduleInfo.verMajor = shared.moduleInfo.verMinor = 0;

    moduleRaw = NULL;
}

//*********************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        shared.logger = &nullLogger;
    else
        shared.logger = loggerIN;
}

//*********************************************************
bool Core::open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW)
{
	const bool SERIAL_IS_BLOCKING = false;

    shared.hCPUServiceChannelW = hCPUServiceChannelW;

    shared.logger->log ("esapi::Core::open\n");
    shared.logger->incIndent();

	shared.logger->log ("com=%s   ", serialPort);
    if (!rhea::rs232::open(&shared.com, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        shared.logger->log ("FAILED. unable to open port [%s]\n", serialPort);
        shared.logger->decIndent();
        return false;
    }
	shared.logger->log ("OK\n");

    //creo la service msg Q sulla quale ricevo msg da thread esterni
    rhea::thread::createMsgQ (&shared.serviceMsgQR, &shared.serviceMsgQW);

    shared.localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");

    //subscriber list
    shared.subscriberList.setup (shared.localAllocator);

    //spawn del modulo raw
    moduleRaw = RHEANEW(shared.localAllocator, esapi::ModuleRaw)();
    if (!moduleRaw->setup (&shared))
    {
        RHEADELETE(shared.localAllocator, moduleRaw);
        moduleRaw = NULL;
	    shared.logger->log ("FAIL\n");
	    shared.logger->decIndent();
    	shared.logger->decIndent();
        return false;
    }

	shared.logger->log ("OK\n");
	shared.logger->decIndent();
    return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (shared.com);

    if (moduleRaw)
    {
        RHEADELETE(shared.localAllocator, moduleRaw);
        moduleRaw = NULL;
    }

    if (shared.localAllocator)
    {
        rhea::thread::deleteMsgQ (shared.serviceMsgQR, shared.serviceMsgQW);

        shared.subscriberList.unsetup();
        RHEADELETE(rhea::getSysHeapAllocator(), shared.localAllocator);
        shared.localAllocator = NULL;
    }
}

//*********************************************************
void Core::run()
{
    //run del module raw
    eExternalModuleType ret = moduleRaw->run();
    RHEADELETE(shared.localAllocator, moduleRaw);
    moduleRaw = NULL;
    shared.logger->log ("esapi::Core::run() => module raw deleted\n");


    //se il module raw ritorna un particolare [eExternalModuleType], allora devo spawnarlo a mandarlo in run, altrimenti
    //vuol dire che abbiamo finito
    switch (ret)
    {
    default:
        break;

    case esapi::eExternalModuleType_rasPI_wifi_REST:
        {
            shared.logger->log ("esapi::Core::run() => spawn moduleRasPI\n");
            ModuleRasPI *m = RHEANEW(shared.localAllocator, ModuleRasPI)();
            m->setup (&shared);
            m->run();
            shared.logger->log ("esapi::Core::run() => deleting moduleRasPI\n");
            RHEADELETE(shared.localAllocator, m);
        }
        break;
    }
    shared.logger->log ("Core::run() => closing...\n");
    priv_close();
}

