#include "CmdHandler_ajaxReq_P0x06_GetAllDecounterValues.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x06_GetAllDecounterValues::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_ALL_DECOUNTER_VALUES (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x06_GetAllDecounterValues::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 decounters[14];
	cpubridge::translateNotify_CPU_ALL_DECOUNTER_VALUES(msgFromCPUBridge, decounters, sizeof(decounters));

	char s[16];
    char resp[256];
	sprintf_s(resp, sizeof(resp), "%d", decounters[0]);
	for (u8 i = 1; i < 14; i++)
	{
		sprintf_s(s, sizeof(s), ",%d", decounters[i]);
		strcat_s(resp, sizeof(resp), s);
	}

	server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
