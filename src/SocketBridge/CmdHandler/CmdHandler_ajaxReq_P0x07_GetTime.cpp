#include "CmdHandler_ajaxReq_P0x07_GetTime.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x07_GetTime::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_TIME (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x07_GetTime::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 hh = 0, mm = 0, ss = 0;
	cpubridge::translateNotify_GET_TIME(msgFromCPUBridge, &hh, &mm, &ss);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"h\":%d,\"m\":%d,\"s\":%d}", hh, mm, ss);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
