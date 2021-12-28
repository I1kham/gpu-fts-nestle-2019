#include "CmdHandler_ajaxReqWiFiGetSSIDList.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqWiFiGetSSIDList::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_WIFI_GET_SSID_LIST (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqWiFiGetSSIDList::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 nSSID;
	const u8 *ssidList;
	cpubridge::translateNotify_WIFI_GET_SSID_LIST (msgFromCPUBridge, &nSSID, &ssidList);
	
	u8 resp[4096];
	rhea::string::utf8::spf (resp, sizeof(resp), "{\"n\":%d,\"list\":\"%s\"}", nSSID, ssidList);
    server->sendAjaxAnwer (hClient, ajaxRequestID, resp, static_cast<u16>(rhea::string::utf8::lengthInBytes(resp)));
}
