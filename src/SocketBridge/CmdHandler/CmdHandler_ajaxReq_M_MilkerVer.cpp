#include "CmdHandler_ajaxReq_M_MilkerVer.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_M_MilkerVer::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_MILKER_VER (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_M_MilkerVer::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char ver[64];
	cpubridge::translateNotify_CPU_MILKER_VER (msgFromCPUBridge, ver, sizeof(ver));
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)ver, (u16)strlen(ver) +1);
}
