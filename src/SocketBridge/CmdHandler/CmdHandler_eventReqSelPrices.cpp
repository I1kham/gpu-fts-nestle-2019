#include "CmdHandler_eventReqSelPrices.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqSelPrices::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_SEL_PRICES (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelPrices::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 prices[NUM_MAX_SELECTIONS];
	u8 numPrices = 0;
	u8 numDecimals = 0;
	cpubridge::translateNotify_CPU_SEL_PRICES_CHANGED(msgFromCPUBridge, &numPrices, &numDecimals, prices);

	//stringa contenente la lista dei prezzi formattati, separati da §
	char strPriceList[512];
	server->formatPriceList (prices, numPrices, numDecimals, strPriceList, sizeof(strPriceList));


    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, strPriceList, (u16)strlen(strPriceList));
}



