#include "CmdHandler_ajaxReq_P0x0B_StatoGruppo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x0B_StatoGruppo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_STATO_GRUPPO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x0B_StatoGruppo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProgrammingCommand_statoGruppo info;
	cpubridge::translateNotify_STATO_GRUPPO(msgFromCPUBridge, &info);
	
	char resp[16];
	if (info == cpubridge::eCPUProgrammingCommand_statoGruppo_nonAttaccato)
		sprintf_s(resp, sizeof(resp), "0");
	else
		sprintf_s(resp, sizeof(resp), "1");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
