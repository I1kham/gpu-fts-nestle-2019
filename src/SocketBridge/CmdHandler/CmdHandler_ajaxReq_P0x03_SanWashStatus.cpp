#include "CmdHandler_ajaxReq_P0x03_SanWashStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x03_SanWashStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x03_SanWashStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 fase = 0;
	u8 btn1 = 0;
	u8 btn2 = 0;
	u8 buffer8[8];
	cpubridge::translateNotify_SAN_WASHING_STATUS(msgFromCPUBridge, &fase, &btn1, &btn2, buffer8);

    char resp[128];
    sprintf_s (resp, sizeof(resp), "{\"fase\":%d,\"btn1\":\"%d\",\"btn2\":\"%d\",\"buffer8\":[%d", fase, btn1, btn2, buffer8[0]);

	for (u8 i = 1; i < 8; i++)
	{
		char s[8];
		sprintf_s (s, sizeof(s), ",%d", buffer8[i]);
		strcat_s (resp, sizeof(resp), s);
	}
	strcat_s (resp, sizeof(resp), "]}");

    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
