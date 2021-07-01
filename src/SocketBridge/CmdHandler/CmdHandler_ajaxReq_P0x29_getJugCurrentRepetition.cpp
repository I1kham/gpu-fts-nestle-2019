#include "CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;



//***********************************************************
void CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_JUG_CURRENT_REPETITION(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 nOf;
	u8 m;
	cpubridge::translateNotify_CPU_GET_JUG_CURRENT_REPETITION(msgFromCPUBridge, &nOf, &m);
    
    char resp[40];
	
	if (nOf && m)
		sprintf_s (resp, sizeof(resp), "{\"numberOf\":%d,\"total\":\"%d\"}", nOf, m);
	else
		sprintf_s (resp, sizeof(resp), "KO");
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}

