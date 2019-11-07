#include "CmdHandler_ajaxReqGetAllDecounterValues.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetAllDecounterValues::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_ALL_DECOUNTER_VALUES (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqGetAllDecounterValues::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 decounters[13];
	cpubridge::translateNotify_CPU_ALL_DECOUNTER_VALUES(msgFromCPUBridge, decounters);

	char s[16];
    char resp[128];
	sprintf_s(resp, sizeof(resp), "%d", decounters[0]);
	for (u8 i = 1; i < 13; i++)
	{
		sprintf_s(s, sizeof(s), ",%d", decounters[i]);
		strcat_s(resp, sizeof(resp), s);
	}


	server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
