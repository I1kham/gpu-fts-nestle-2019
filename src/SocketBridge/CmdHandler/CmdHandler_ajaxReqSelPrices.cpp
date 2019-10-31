#include "CmdHandler_ajaxReqSelPrices.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqSelPrices::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_SEL_PRICES (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqSelPrices::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 prices[NUM_MAX_SELECTIONS];
	cpubridge::translateNotify_CPU_SEL_PRICES_CHANGED (msgFromCPUBridge, prices, sizeof(prices));

    //stringa contenente la lista dei prezzi formattati, separati da §
	char strPriceList[512];
	server->formatPriceList(prices, NUM_MAX_SELECTIONS, strPriceList, sizeof(strPriceList));

	
	char resp[512];
	sprintf_s(resp, sizeof(resp), "{\"n\":%d,\"s\":\"%s\"}", NUM_MAX_SELECTIONS, strPriceList);
    server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
