#include "CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_CUPSENSOR_LIVE_VALUE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 val = 0;
	cpubridge::translateNotify_CPU_GET_CUPSENSOR_LIVE_VALUE(msgFromCPUBridge, &val);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "%d", val);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
