#include "CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 fase = 0;
	u8 esito = 0;
	u16 results_number[2];
	cpubridge::translateNotify_GET_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(msgFromCPUBridge, &fase, &esito, &results_number[0], &results_number[1]);

	char results[2][16];
	for (u8 i = 0; i < 2; i++)
	{
		memset(results[i], 0, sizeof(results[i]));
		const u16 parte_intera = results_number[i] / 100;
		const u16 parte_decimale = results_number[i] - parte_intera* 100;
		sprintf_s(results[i], sizeof(results[i]), "%d.%d", parte_intera, parte_decimale);
	}

    char resp[1024];
    sprintf_s (resp, sizeof(resp), "{\"fase\":%d,\"esito\":\"%d\",\"r1up\":\"%s\",\"r1down\":\"%s\"}", fase, esito, results[0], results[1]);
    server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
