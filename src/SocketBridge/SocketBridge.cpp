#include "SocketBridge.h"
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"


struct sServerInitParam
{
    rhea::ISimpleLogger *logger;
	OSEvent				hEvThreadStarted;
	HThreadMsgW			hCPUServiceChannelW;
	u8					instanceNumber;
};

struct sServerInstance
{
	rhea::HThread		hThread;
	socketbridge::Server *server;
};

#define N_MAX_SOCKET_BRIDGE_SERVER_INSTANCE	4
sServerInstance socketBridgeServerInstances[N_MAX_SOCKET_BRIDGE_SERVER_INSTANCE];
u8				socketBridgeServerInstances_ct = 0;


i16     serverThreadFn (void *userParam);



/**************************************************************************
 * startServer
 *
 */
bool socketbridge::startServer (rhea::ISimpleLogger *logger, const HThreadMsgW &hCPUServiceChannelW, rhea::HThread *out_hThread)
{
    sServerInitParam    init;
	init.instanceNumber = socketBridgeServerInstances_ct++;
	

    //crea il thread del server
    init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	rhea::event::open (&init.hEvThreadStarted);
    rhea::thread::create (out_hThread, serverThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = rhea::event::wait(init.hEvThreadStarted, 3000);
	rhea::event::close(init.hEvThreadStarted);

	socketBridgeServerInstances[init.instanceNumber].hThread = *out_hThread;

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
		socketBridgeServerInstances[init->instanceNumber].server = &server;
				
		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();
	return 1;
}


//*****************************************************************
socketbridge::Server* socketbridge::priv_getInstanceFromHThread(const rhea::HThread hThread)
{
	for (u8 i = 0; i < socketBridgeServerInstances_ct; i++)
	{
		if (socketBridgeServerInstances[i].hThread == hThread)
			return socketBridgeServerInstances[i].server;
	}
	return NULL;
}