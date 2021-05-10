#include "CmdHandler_ajaxReq_P0x25_caffeCortesia.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x25_caffeCortesia::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_RUN_CAFFE_CORTESIA(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x25_caffeCortesia::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char resp[8];
	sprintf_s(resp, sizeof(resp), "OK");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
