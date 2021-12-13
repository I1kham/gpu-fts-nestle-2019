#include "ESAPICore.h"
#include "ESAPI.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "ESAPIModuleRaw.h"
#include "ESAPIModuleRasPI.h"
#include "ESAPIProtocol.h"

using namespace esapi;

//*********************************************************
Core::Core()
{
	shared.localAllocator = NULL;
	shared.protocol = NULL;
    shared.logger = &nullLogger;
    bIsSubscribedToCPUBridge = false;
    shared.moduleInfo.type = esapi::eExternalModuleType::none;
    shared.moduleInfo.verMajor = shared.moduleInfo.verMinor = 0;
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
    shared.localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");


    shared.logger->log ("esapi::Core::open\n");
    shared.logger->incIndent();

    //iscrizione a CPUBridge
    shared.logger->log ("subscribing to CPUBridge...");
    if (!priv_subscribeToCPUBridge (hCPUServiceChannelW))
    {
        shared.logger->log ("FAIL\n");
        shared.logger->decIndent();
        shared.logger->decIndent();
        return false;
    }
    bIsSubscribedToCPUBridge = true;
    shared.logger->log ("OK\n");

    //creo la service msg Q sulla quale ricevo msg da thread esterni
    rhea::thread::createMsgQ (&shared.serviceMsgQR, &shared.serviceMsgQW);
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(shared.serviceMsgQR, &h);
        shared.waitableGrp.addEvent(h, ESAPI_WAITABLEGRP_EVENT_FROM_SERVICE_MSGQ);
    }

    //subscriber list
    shared.subscriberList.setup (shared.localAllocator);

    //creazione "protocol"
    shared.logger->log ("instancing 'protocol'...\n");
    shared.logger->incIndent();
    shared.protocol = RHEANEW(shared.localAllocator, esapi::Protocol)();
    if (!shared.protocol->setup(shared.localAllocator, &shared.cpuBridgeSubscriber, serialPort, &shared.waitableGrp, shared.logger))
    {
        shared.logger->log ("FAIL\n");
        shared.logger->decIndent();
        shared.logger->decIndent();
        return false;
    }
    shared.logger->log ("OK\n");
    shared.logger->decIndent();

    shared.logger->decIndent();
    return true;
}

//*********************************************************
bool Core::priv_subscribeToCPUBridge(const HThreadMsgW &hCPUServiceChannelW)
{
    //creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
    HThreadMsgR hTempMsgQR;
    HThreadMsgW hTempMsgQW;
    rhea::thread::createMsgQ (&hTempMsgQR, &hTempMsgQW);

    //mando la richiesta
    cpubridge::subscribe (hCPUServiceChannelW, hTempMsgQW, ESAPI_SUBSCRIBER_UID);

    //attendo risposta
    bool bSubscribed = false;
    u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
    do
    {
        rhea::thread::sleepMSec(50);

        rhea::thread::sMsg msg;
        if (rhea::thread::popMsg(hTempMsgQR, &msg))
        {
            //ok, ci siamo
            if (msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER)
            {
                u8 cpuBridgeVersion = 0;
                cpubridge::translate_SUBSCRIPTION_ANSWER(msg, &shared.cpuBridgeSubscriber, &cpuBridgeVersion);
                rhea::thread::deleteMsg(msg);

                OSEvent h;
                rhea::thread::getMsgQEvent(shared.cpuBridgeSubscriber.hFromMeToSubscriberR, &h);
                shared.waitableGrp.addEvent (h, ESAPI_WAITABLEGRP_EVENT_FROM_CPUBRIDGE);
                bSubscribed = true;
                break;
            }

            rhea::thread::deleteMsg(msg);
        }
    } while (rhea::getTimeNowMSec() < timeToExitMSec);

    //delete della msgQ
    rhea::thread::deleteMsgQ (hTempMsgQR, hTempMsgQW);
    return bSubscribed;
}

//*****************************************************************
void Core::priv_close ()
{
    if (NULL == shared.localAllocator)
        return;

    //rimuovo la serviceMsgQ dalla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(shared.serviceMsgQR, &h);
        shared.waitableGrp.removeEvent(h);
    }

    if (bIsSubscribedToCPUBridge)
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(shared.cpuBridgeSubscriber.hFromMeToSubscriberR, &h);
        shared.waitableGrp.removeEvent (h);
        cpubridge::unsubscribe(shared.cpuBridgeSubscriber);
        bIsSubscribedToCPUBridge = false;
    }

    if (shared.protocol)
    {
        RHEADELETE(shared.localAllocator, shared.protocol);
        shared.protocol = NULL;
    }
	
    rhea::thread::deleteMsgQ (shared.serviceMsgQR, shared.serviceMsgQW);

    shared.subscriberList.unsetup();
    RHEADELETE(rhea::getSysHeapAllocator(), shared.localAllocator);
    shared.localAllocator = NULL;
}

//*********************************************************
void Core::run()
{
    //spawn del modulo raw
    shared.logger->log ("esapi::Core::run() => instancing module_raw\n");
    Module	*currentModule = RHEANEW(shared.localAllocator, esapi::ModuleRaw)();
    currentModule->setup (&shared);

    bool bQuit = false;
    while (bQuit == false)
    {
        sShared::eRetCode ret = shared.run (currentModule);
        RHEADELETE(shared.localAllocator, currentModule);
        currentModule = NULL;

        switch (ret)
        {
        default:
            bQuit = true;
            break;

        case sShared::eRetCode::START_MODULE_RASPI:
            {
                shared.logger->log ("esapi::Core::run() => instancing moduleRasPI\n");
                ModuleRasPI *m = RHEANEW(shared.localAllocator, ModuleRasPI)();
                m->setup (&shared);
                currentModule = m;
            }
            break;
        }
    }
    shared.logger->log ("esapi::Core::run() => closing...\n");
    priv_close();
}

