#include "CmdHandler_eventReq_AliChina_onlineStatus.h"
#include "../SocketBridge.h"

using namespace socketbridge;


//***********************************************************
void CmdHandler_eventReq_AliChina_onlineStatus::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient)
{
	u8 buffer[2];
	if (server->module_alipayChina_isOnline())
		buffer[0] = 1;
	else
		buffer[0] = 0;

    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, 1);
}