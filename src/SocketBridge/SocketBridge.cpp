#include "SocketBridge.h"
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"


struct sServerInitParam
{
    rhea::ISimpleLogger *logger;
	OSEvent				hEvThreadStarted;
	HThreadMsgW			hCPUServiceChannelW;
};

i16     serverThreadFn (void *userParam);



/**************************************************************************
 * startServer
 *
 */
bool socketbridge::startServer (rhea::ISimpleLogger *logger, const HThreadMsgW &hCPUServiceChannelW, rhea::HThread *out_hThread)
{
    bool ret = true;
    sServerInitParam    init;

    //crea il thread del server
    init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	OSEvent_open (&init.hEvThreadStarted);
    rhea::thread::create (out_hThread, serverThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = OSEvent_wait(init.hEvThreadStarted, 3000);
	OSEvent_close(init.hEvThreadStarted);

	return bStarted;
}

//*****************************************************************
i16 serverThreadFn (void *userParam)
{
	sServerInitParam *init = (sServerInitParam*)userParam;

	socketbridge::Server server;
	server.useLogger (init->logger);
	if (server.open (2280, init->hCPUServiceChannelW))
	{
		//segnalo che il thread è partito con successo
		OSEvent_fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();
	return 1;
}

