#include "CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char text[4] = { 'O', 'K', 0, 0 };
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
