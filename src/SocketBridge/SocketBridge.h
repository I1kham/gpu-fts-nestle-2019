#ifndef _SocketBridge_h_
#define _SocketBridge_h_
#include "SocketBridgeServer.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"



namespace socketbridge
{
	bool        startServer (rhea::ISimpleLogger *logger, const HThreadMsgW &hCPUServiceChannelW, rhea::HThread *out_hThread);
				/*	crea il thread che monitora e gestisce la socket.
					Se hCPUServiceChannelW è valido, allora thread in questione si "subscribe()" al thread della CPU in modo da riceverne le notifiche
				*/
} // namespace socketbridge


#endif // _SocketBridge_h_

