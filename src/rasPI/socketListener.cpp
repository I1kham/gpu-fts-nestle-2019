#include "socketListener.h"
#include "socketListenerCore.h"

using namespace rasPI;


struct sRasPISokListenerInitParam
{
    rhea::ISimpleLogger *logger;
	OSEvent				hEvThreadStarted;
};


static socketListener::Core *rasPISokListenerCore = NULL;


i16     rasPISokListenerThreadFn (void *userParam);

//****************************************************
bool socketListener::start (rhea::ISimpleLogger *logger, rhea::HThread *out_hThread)
{
    sRasPISokListenerInitParam    init;
	
    //crea il thread del Core
    init.logger = logger;
	rhea::event::open (&init.hEvThreadStarted);
    rhea::thread::create (out_hThread, rasPISokListenerThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = rhea::event::wait(init.hEvThreadStarted, 3000);
	rhea::event::close(init.hEvThreadStarted);

	return bStarted;
}

//*****************************************************************
i16 rasPISokListenerThreadFn (void *userParam)
{
    sRasPISokListenerInitParam *init = static_cast<sRasPISokListenerInitParam*>(userParam);

    rasPISokListenerCore = RHEANEW(rhea::getSysHeapAllocator(), socketListener::Core)();
    rasPISokListenerCore->useLogger (init->logger);
    if (rasPISokListenerCore->open (2280))
	{
        //segnalo che il thread e' partito con successo
		rhea::event::fire(init->hEvThreadStarted);
        rasPISokListenerCore->run();
	}
    RHEADELETE(rhea::getSysHeapAllocator(), rasPISokListenerCore);
	return 1;
}