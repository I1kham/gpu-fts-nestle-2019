#include "CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_GRUPPO(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 fase = 0;
	u8 esito = 0;
	u16 results_number[12];
	cpubridge::translateNotify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(msgFromCPUBridge, &fase, &esito, results_number);

	char results[12][16];
	for (u8 i = 0; i < 12; i++)
	{
		memset(results[i], 0, sizeof(results[i]));
		const u16 parte_intera = results_number[i] / 100;
		const u16 parte_decimale = results_number[i] - parte_intera* 100;
		sprintf_s(results[i], sizeof(results[i]), "%d.%d", parte_intera, parte_decimale);
	}

    char resp[1024];
    sprintf_s (resp, sizeof(resp), "{\"fase\":%d,\"esito\":\"%d\",\
\"r1up\":\"%s\",\"r1down\":\"%s\",\
\"r2up\":\"%s\",\"r2down\":\"%s\",\
\"r3up\":\"%s\",\"r3down\":\"%s\",\
\"r4up\":\"%s\",\"r4down\":\"%s\",\
\"r5up\":\"%s\",\"r5down\":\"%s\",\
\"r6up\":\"%s\",\"r6down\":\"%s\"}",\
		fase, esito, results[0], results[1], results[2], results[3], results[4], results[5], results[6], results[7], results[8], results[9], results[10], results[11]);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
