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