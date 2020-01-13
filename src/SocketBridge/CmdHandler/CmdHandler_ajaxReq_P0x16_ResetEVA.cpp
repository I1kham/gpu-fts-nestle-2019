#include "CmdHandler_ajaxReq_P0x16_ResetEVA.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x16_ResetEVA::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_EVA_RESET_PARTIALDATA(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x16_ResetEVA::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool result = false;
	cpubridge::translateNotify_EVA_RESET_PARTIALDATA(msgFromCPUBridge, &result);
	char resp[8];
	if (result)
		sprintf(resp, "OK");
	else
		sprintf(resp, "KO");

	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}

