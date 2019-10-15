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
    sServerInitParam    init;

    //crea il thread del server
    init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	rhea::event::open (&init.hEvThreadStarted);
    rhea::thread::create (out_hThread, serverThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = rhea::event::wait(init.hEvThreadStarted, 3000);
	rhea::event::close(init.hEvThreadStarted);

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
		rhea::event::fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();
	return 1;
}



