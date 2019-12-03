#include "CmdHandler_ajaxReqResetEVA.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqResetEVA::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_NOMI_LINGE_CPU(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqResetEVA::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char resp[8];
	//if (rhea::fs::fileCopy(src, dst))
		sprintf(resp, "OK");

	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}

