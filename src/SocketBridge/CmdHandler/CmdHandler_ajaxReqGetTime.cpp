#include "CmdHandler_ajaxReqGetTime.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetTime::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_TIME (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqGetTime::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 hh = 0, mm = 0, ss = 0;
	cpubridge::translateNotify_GET_TIME(msgFromCPUBridge, &hh, &mm, &ss);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"h\":%d,\"m\":%d,\"s\":%d}", hh, mm, ss);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
