#include "CmdHandler_ajaxReq_P0x1C_StartModemTest.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1C_StartModemTest::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_START_MODEM_TEST (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1C_StartModemTest::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char text[4] = { 'O', 'K', 0, 0 };
	server->sendAjaxAnwer(hClient, ajaxRequestID, text, (u16)strlen(text));
}
