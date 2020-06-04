#include "CmdHandler_eventReqRasPI_getIPandSSID.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqRasPI_getIPandSSID::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_RASPI_MITM_GET_WIFI_IPandSSID (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqRasPI_getIPandSSID::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
    char ipAddress[32], ssid[256];
    memset (ipAddress, 0, sizeof(ipAddress));
    memset (ssid, 0, sizeof(ssid));
    cpubridge::translateNotify_CPU_RASPI_MITM_GET_WIFI_IPandSSID (msgFromCPUBridge, ipAddress, sizeof(ipAddress), ssid, sizeof(ssid));

	char resp[512];
	sprintf_s(resp, sizeof(resp), "{\"ip\":\"%s\",\"ssid\":\"%s\"}", ipAddress, ssid);
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
