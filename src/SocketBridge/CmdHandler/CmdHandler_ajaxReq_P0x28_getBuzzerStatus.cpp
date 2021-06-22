#include "CmdHandler_ajaxReq_P0x28_getBuzzerStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x28_getBuzzerStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_BUZZER_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x28_getBuzzerStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	bool bBuzzerBusy = false;;
	cpubridge::translateNotify_CPU_BUZZER_STATUS(msgFromCPUBridge, &bBuzzerBusy);

	char resp[64];
	if (bBuzzerBusy)
		sprintf_s(resp, sizeof(resp), "RUN");
	else
		sprintf_s(resp, sizeof(resp), "IDLE");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
