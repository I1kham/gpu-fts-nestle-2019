#include "CmdHandler_ajaxReq_P0x1D_ResetEVATotals.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1D_ResetEVATotals::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_EVA_RESET_TOTALS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1D_ResetEVATotals::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char resp[8];
	sprintf(resp, "OK");
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}

