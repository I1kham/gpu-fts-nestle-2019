#include "CmdHandler_ajaxReq_snack_0x04_enterProg.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_snack_0x04_enterProg::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_SNACK_ENTER_PROG(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_snack_0x04_enterProg::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool result;
	cpubridge::translateNotify_SNACK_ENTER_PROG(msgFromCPUBridge, &result);
	
	char resp[16];
	if (result)
		sprintf_s(resp, sizeof(resp), "OK");
	else
		sprintf_s(resp, sizeof(resp), "KO");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
