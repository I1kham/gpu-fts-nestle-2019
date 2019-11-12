#include "CmdHandler_ajaxReqStatoGruppo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqStatoGruppo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_STATO_GRUPPO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqStatoGruppo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProgrammingCommand_statoGruppo info;
	cpubridge::translateNotify_STATO_GRUPPO(msgFromCPUBridge, &info);
	
	char resp[16];
	if (info == cpubridge::eCPUProgrammingCommand_statoGruppo_nonAttaccato)
		sprintf_s(resp, sizeof(resp), "0");
	else
		sprintf_s(resp, sizeof(resp), "1");
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
