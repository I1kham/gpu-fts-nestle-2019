#include "CmdHandler_ajaxReqGetDate.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetDate::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_DATE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqGetDate::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 year = 0;
	u8 month = 0;
	u8 day = 0;
	cpubridge::translateNotify_GET_DATE(msgFromCPUBridge, &year, &month, &day);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"y\":%d,\"m\":%d,\"d\":%d}", year, month, day);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
