#include "SocketBridge.h"
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"

struct sServerInitParam
{
    rhea::ISimpleLogger *logger;
	OSEvent				hEvThreadStarted;
	HThreadMsgW			hCPUServiceChannelW;
	bool				bDieWhenNoClientConnected;
};


static socketbridge::Server *serverInstance = NULL;


i16     serverThreadFn (void *userParam);


//*****************************************************************************
bool socketbridge_helper_folder_create (const char *folder, rhea::ISimpleLogger *logger)
{
    char s[512];
    sprintf_s(s, sizeof(s), "%s/%s", rhea::getPhysicalPathToAppFolder(), folder);
    if (!rhea::fs::folderCreate(s))
    {
        logger->log ("ERR: can't create folder [%s]\n", s);
        return false;
    }
    else
    {
        logger->log ("CREATED folder [%s]\n", s);
        return true;
    }
}


/**************************************************************************
 * startServer
 *
 */
bool socketbridge::startServer (rhea::ISimpleLogger *logger, const HThreadMsgW &hCPUServiceChannelW, bool bDieWhenNoClientConnected, rhea::HThread *out_hThread)
{
    sServerInitParam    init;
	
	//creo la struttura di cartelle necessarie al corretto funzionamento
    //socketbridge_helper_folder_create("current/socketbridge", logger);


    //crea il thread del server
    init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	init.bDieWhenNoClientConnected = bDieWhenNoClientConnected;
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
    //sServerInitParam *init = (sServerInitParam*)userParam;
    sServerInitParam *init = static_cast<sServerInitParam*>(userParam);

    serverInstance = RHEANEW(rhea::memory_getDefaultAllocator(), socketbridge::Server)();
    serverInstance->useLogger (init->logger);
    if (serverInstance->open (2280, init->hCPUServiceChannelW, init->bDieWhenNoClientConnected ))
	{
        //segnalo che il thread e' partito con successo
		rhea::event::fire(init->hEvThreadStarted);
        serverInstance->run();
	}
    serverInstance->close();
    RHEADELETE(rhea::memory_getDefaultAllocator(), serverInstance);
	return 1;
}


//*****************************************************************
socketbridge::Server* socketbridge::priv_getInstanceFromHThread(const rhea::HThread hThread UNUSED_PARAM)
{
    return serverInstance;
}
