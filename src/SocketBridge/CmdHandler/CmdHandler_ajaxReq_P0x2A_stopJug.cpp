#include "CmdHandler_ajaxReq_P0x2A_stopJug.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;



//***********************************************************
void CmdHandler_ajaxReq_P0x2A_stopJug::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_STOP_JUG(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x2A_stopJug::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bYes;
	cpubridge::translateNotify_CPU_STOP_JUG(msgFromCPUBridge, &bYes);
    
    char resp[8];
	if (bYes)
		sprintf_s (resp, sizeof(resp), "OK");
	else
		sprintf_s (resp, sizeof(resp), "KO");
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}

