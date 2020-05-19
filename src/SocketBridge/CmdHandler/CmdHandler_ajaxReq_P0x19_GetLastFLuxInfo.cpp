#include "CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_LAST_FLUX_INFORMATION(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 lastFlux = 0;
	u16 lastGrinderPosition = 0;
	cpubridge::translateNotify_GET_LAST_FLUX_INFORMATION(msgFromCPUBridge, &lastFlux, &lastGrinderPosition);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"flux\":%d,\"grinderPos\":%d}", lastFlux, lastGrinderPosition);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
