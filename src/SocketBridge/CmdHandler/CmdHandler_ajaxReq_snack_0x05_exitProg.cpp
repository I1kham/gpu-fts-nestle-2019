#include "CmdHandler_ajaxReq_snack_0x05_exitProg.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_snack_0x05_exitProg::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_SNACK_EXIT_PROG(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_snack_0x05_exitProg::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool result;
	cpubridge::translateNotify_SNACK_EXIT_PROG(msgFromCPUBridge, &result);
	
	char resp[16];
	if (result)
		sprintf_s(resp, sizeof(resp), "OK");
	else
		sprintf_s(resp, sizeof(resp), "KO");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
