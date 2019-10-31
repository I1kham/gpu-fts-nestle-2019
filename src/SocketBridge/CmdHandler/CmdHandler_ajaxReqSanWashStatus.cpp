#include "CmdHandler_ajaxReqSanWashStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqSanWashStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_SANWASH_STATUS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqSanWashStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 fase = 0;
	u8 btn1 = 0;
	u8 btn2 = 0;
	cpubridge::translateNotify_SAN_WASHING_STATUS(msgFromCPUBridge, &fase, &btn1, &btn2);

    char resp[64];
    sprintf_s (resp, sizeof(resp), "{\"fase\":%d,\"btn1\":\"%d\",\"btn2\":\"%d\"}", fase, btn1, btn2);
    server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
