#include "CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_STATO_CALCOLO_IMPULSI_GRUPPO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 stato = 0;
	u16 valore = 0;
	cpubridge::translateNotify_STATO_CALCOLO_IMPULSI_GRUPPO(msgFromCPUBridge, &stato, &valore);

    char resp[64];
    sprintf_s (resp, sizeof(resp), "{\"s\":%d,\"v\":\"%d\"}", stato, valore);
    server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
