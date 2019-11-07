#include "CmdHandler_ajaxReqmachineTypeAndModel.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqmachineTypeAndModel::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqmachineTypeAndModel::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::sExtendedCPUInfo info;
	cpubridge::translateNotify_EXTENDED_CONFIG_INFO(msgFromCPUBridge, &info);
	
	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"mType\":%d,\"mModel\":%d}", (u8)info.machineType, info.machineModel);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
