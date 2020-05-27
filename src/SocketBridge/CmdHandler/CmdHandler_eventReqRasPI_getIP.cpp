#include "CmdHandler_eventReqRasPI_getIP.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqRasPI_getIP::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_RASPI_MITM_GET_WIFI_IP (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqRasPI_getIP::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
    char ipAddress[256];
    memset (ipAddress, 0, sizeof(ipAddress));
    cpubridge::translateNotify_CPU_RASPI_MITM_GET_WIFI_IP (msgFromCPUBridge, ipAddress, sizeof(ipAddress));
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)ipAddress, (u16)strlen(ipAddress) +1);
}
