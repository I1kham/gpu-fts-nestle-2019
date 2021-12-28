#include "CmdHandler_ajaxReqNetworkSettings.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqNetworkSettings::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_NETWORK_SETTINGS(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqNetworkSettings::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::sNetworkSettings info;
	cpubridge::translateNotify_NETWORK_SETTINGS(msgFromCPUBridge, &info);
	
	char resp[512];
	sprintf_s(resp, sizeof(resp), "{\"lanIP\":\"%s\",\"wifiIP\":\"%s\",\"mac\":\"%s\",\"hotSpotSSID\":\"%s\",\"modemLTE\":%d,\"wifiMode\":%d,\"wifiSSID\":\"%s\",\"wifiPwd\":\"%s\",\"wifiConnected\":%d}", 
			info.lanIP, info.wifiIP, info.macAddress,
		info.wifiHotSpotSSID, info.isModemLTEEnabled, static_cast<u8>(info.wifiMode),
		info.wifiConnectTo_SSID, info.wifiConnectTo_pwd, info.wifiConnectTo_isConnected
	);
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
