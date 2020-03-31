#include "CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_GRUPPO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char text[4] = { 'O', 'K', 0, 0 };
	server->sendAjaxAnwer(hClient, ajaxRequestID, text, (u16)strlen(text));
}
