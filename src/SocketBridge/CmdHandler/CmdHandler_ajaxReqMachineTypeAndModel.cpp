#include "CmdHandler_ajaxReqMachineTypeAndModel.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqMachineTypeAndModel::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqMachineTypeAndModel::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::sExtendedCPUInfo info;
	cpubridge::translateNotify_EXTENDED_CONFIG_INFO(msgFromCPUBridge, &info);
	
	u8 alipayChinaActive = 0;
	if (server->module_alipayChina_hasBeenActivated())
		alipayChinaActive = 1;

	char resp[128];
	sprintf_s(resp, sizeof(resp), "{\"mType\":%d,\"mModel\":%d,\"isInduzione\":%d,\"gruppo\":\"%c\",\"aliChina\":%d}", (u8)info.machineType, info.machineModel, info.isInduzione, (char)info.tipoGruppoCaffe, alipayChinaActive);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
