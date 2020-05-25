#include "rasPIMITM.h"
#include "rasPIMITMCore.h"

using namespace rasPI;

struct sRasPIMITIMInitParam
{
    rhea::ISimpleLogger *logger;
    const char          *serialPortGPU;
    const char          *serialPortCPU;
	OSEvent				hEvThreadStarted;

    HThreadMsgW         returned_msgQW_toMITM;
};


static rasPI::MITM::Core *rasPIMITMCore = NULL;


i16     rasPIMITMThreadFn (void *userParam);

//****************************************************
bool MITM::start (rhea::ISimpleLogger *logger, const char *serialPortGPU, const char *serialPortCPU, rhea::HThread *out_hThread, HThreadMsgW *out_msgQW_toMITM)
{
    sRasPIMITIMInitParam    init;
	
    //crea il thread del Core
    init.logger = logger;
    init.serialPortGPU = serialPortGPU;
    init.serialPortCPU = serialPortCPU;
	rhea::event::open (&init.hEvThreadStarted);
    rhea::thread::create (out_hThread, rasPIMITMThreadFn, &init);

	//attendo che il thread sia partito
	bool bStarted = rhea::event::wait(init.hEvThreadStarted, 3000);
	rhea::event::close(init.hEvThreadStarted);

    *out_msgQW_toMITM = init.returned_msgQW_toMITM;

	return bStarted;
}

//*****************************************************************
i16 rasPIMITMThreadFn (void *userParam)
{
    sRasPIMITIMInitParam *init = static_cast<sRasPIMITIMInitParam*>(userParam);

    rasPIMITMCore = RHEANEW(rhea::getSysHeapAllocator(), MITM::Core)();
    rasPIMITMCore->useLogger (init->logger);
    if (rasPIMITMCore->open (init->serialPortGPU, init->serialPortCPU))
	{
        init->returned_msgQW_toMITM = rasPIMITMCore->getMsgQW();

        //segnalo che il thread e' partito con successo
		rhea::event::fire(init->hEvThreadStarted);
        rasPIMITMCore->run();
	}
    RHEADELETE(rhea::getSysHeapAllocator(), rasPIMITMCore);
	return 1;
}

/****************************************************
 * é lo stesso identico meccanismo usato da socketBridge per iscriversi a CPUBridge.
 * Vedi socketBridgeServer::priv_subsribeToCPU()
 */
bool MITM::subscribe(const HThreadMsgW &msgQW_toMITM, cpubridge::sSubscriber *out)
{
	//creo una msgQ temporanea per ricevere da MITM la risposta alla mia richiesta di iscrizione
	HThreadMsgR hMsgQR;
	HThreadMsgW hMsgQW;
	rhea::thread::createMsgQ (&hMsgQR, &hMsgQW);

	//invio la richiesta a MITM
	//cpubridge::subscribe (hCPUServiceChannelW, hMsgQW);
	u32 param32 = hMsgQW.asU32();
	rhea::thread::pushMsg (msgQW_toMITM, CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST, param32);

	//attendo risposta
	bool ret = false;
	const u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
	do
	{
		rhea::thread::sleepMSec(50);

		rhea::thread::sMsg msg;
		if (rhea::thread::popMsg(hMsgQR, &msg))
		{
			//ok, ci siamo
			if (msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER)
			{
				//cpubridge::translate_SUBSCRIPTION_ANSWER (msg, &subscriber, &cpuBridgeVersion);
				assert(msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER);
				memcpy(out, msg.buffer, sizeof(cpubridge::sSubscriber));

				rhea::thread::deleteMsg(msg);
				ret = true;
				break;
			}

			rhea::thread::deleteMsg(msg);
		}
	} while (rhea::getTimeNowMSec() < timeToExitMSec);
	
	//delete della msgQ
	rhea::thread::deleteMsgQ (hMsgQR, hMsgQW);
	
	return ret;
}