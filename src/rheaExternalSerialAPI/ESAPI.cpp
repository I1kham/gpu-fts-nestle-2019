#include "ESAPI.h"
#include "ESAPICore.h"
#include "../rheaCommonLib/rheaUtils.h"


struct sEsapiThreadInitParam
{
	rhea::ISimpleLogger *logger;
	char				comPort[64];
	HThreadMsgW			hCPUServiceChannelW;
	OSEvent				hEvThreadStarted;
};


i16     esapiThreadFn (void *userParam);

//****************************************************************************
bool esapi::startThread (const char *comPort, const HThreadMsgW &hCPUServiceChannelW, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread)
{
	sEsapiThreadInitParam    init;

	//crea il thread
	init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	sprintf_s (init.comPort, sizeof(init.comPort), "%s", comPort);
	rhea::event::open (&init.hEvThreadStarted);
	rhea::thread::create (out_hThread, esapiThreadFn, &init);

	//attendo che il thread sia partito
	bool bStarted = rhea::event::wait (init.hEvThreadStarted, 1000);
	rhea::event::close(init.hEvThreadStarted);

	if (bStarted)
	{
		return true;
	}

	return false;
}

//*****************************************************************
i16 esapiThreadFn (void *userParam)
{
	sEsapiThreadInitParam *init = (sEsapiThreadInitParam*)userParam;

	esapi::Core core;
	core.useLogger(init->logger);
	if (core.open (init->comPort, init->hCPUServiceChannelW))
	{
		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		core.run();
	}

	return 1;
}
//****************************************************************************
bool esapi::isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse)
{
	return (ck == rhea::utils::simpleChecksum8_calc(buffer, numBytesToUse));
}
