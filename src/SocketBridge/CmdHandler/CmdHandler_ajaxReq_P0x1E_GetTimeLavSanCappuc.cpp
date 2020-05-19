#include "CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 hh = 0, mm = 0;
	cpubridge::translateNotify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(msgFromCPUBridge, &hh, &mm);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"h\":%d,\"m\":%d}", hh, mm);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
