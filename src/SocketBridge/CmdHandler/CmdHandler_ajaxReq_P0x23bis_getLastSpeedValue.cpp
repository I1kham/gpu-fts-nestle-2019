#include "CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_LAST_GRINDER_SPEED(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 speed = 0;
	cpubridge::translateNotify_CPU_GET_LAST_GRINDER_SPEED(msgFromCPUBridge, &speed);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "%d", speed);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
