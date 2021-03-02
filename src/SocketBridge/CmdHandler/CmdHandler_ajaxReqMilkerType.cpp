#include "CmdHandler_ajaxReqMilkerType.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqMilkerType::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_MILKER_TYPE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqMilkerType::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUMilkerType milkerType;
	cpubridge::translateNotify_MILKER_TYPE(msgFromCPUBridge, &milkerType);
	
	char resp[32];
	sprintf_s(resp, sizeof(resp), "{\"mType\":%d}", (u8)milkerType);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
