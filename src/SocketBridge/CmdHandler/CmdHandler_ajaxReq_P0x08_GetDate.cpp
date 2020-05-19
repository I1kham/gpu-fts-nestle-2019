#include "CmdHandler_ajaxReq_P0x08_GetDate.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x08_GetDate::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_DATE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x08_GetDate::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 year = 0;
	u8 month = 0;
	u8 day = 0;
	cpubridge::translateNotify_GET_DATE(msgFromCPUBridge, &year, &month, &day);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"y\":%d,\"m\":%d,\"d\":%d}", year, month, day);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
