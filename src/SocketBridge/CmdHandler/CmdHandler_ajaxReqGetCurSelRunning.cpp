#include "CmdHandler_ajaxReqGetCurSelRunning.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetCurSelRunning::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_CUR_SEL_RUNNING(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqGetCurSelRunning::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 selNum;
	cpubridge::translateNotify_CPU_CUR_SEL_RUNNING(msgFromCPUBridge, &selNum);
    
    char resp[64];
	sprintf_s(resp, sizeof(resp), "%d", selNum);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
