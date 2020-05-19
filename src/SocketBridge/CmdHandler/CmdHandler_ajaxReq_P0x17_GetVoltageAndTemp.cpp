#include "CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_VOLT_AND_TEMP (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 tCamera = 0;
	u8 tBollitore = 0;
	u8 tCappuccinatore = 0;
	u16 voltaggio = 0;
	cpubridge::translateNotify_GET_VOLT_AND_TEMP(msgFromCPUBridge, &tCamera, &tBollitore, &tCappuccinatore, &voltaggio);

	char resp[64];
	sprintf_s(resp, sizeof(resp), "{\"tcam\":%d,\"tbol\":%d,\"tcap\":%d,\"v\":%d}", tCamera, tBollitore, tCappuccinatore, voltaggio);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
