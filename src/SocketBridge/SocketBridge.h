#ifndef _SocketBridge_h_
#define _SocketBridge_h_
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/Protocol/IProtocolChannell.h"
#include "../rheaCommonLib/Protocol/IProtocol.h"


namespace socketbridge
{
	bool        startServer (rhea::ISimpleLogger *logger, const HThreadMsgW &hCPUServiceChannelW, rhea::HThread *out_hThread);
				/*	crea il thread che monitora e gestisce la socket.
					Se hCPUServiceChannelW è valido, allora thread in questione si "subscribe()" al thread della CPU in modo da riceverne le notifiche
				*/
					
	Server*		priv_getInstanceFromHThread (const rhea::HThread hThread);



				template<class TTask>
	void		addTask (const rhea::HThread hThread, const char *taskName) 
				{ 
					priv_getInstanceFromHThread(hThread)->taskAdd<TTask>(taskName);
				}

} // namespace socketbridge


#endif // _SocketBridge_h_

