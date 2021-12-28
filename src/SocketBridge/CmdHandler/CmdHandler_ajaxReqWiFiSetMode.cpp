#include "CmdHandler_ajaxReqWiFiSetMode.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	cpubridge::eWifiMode	mode;
	u8						ssid[128];
	u8						pwd[128];
};

//***********************************************************
bool ajaxReqWiFiSetMode_jsonTrapFunction(const u8 *const fieldName, const u8 *const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "mode") == 0)
	{
		switch (rhea::string::utf8::toU32(fieldValue))
		{
		default:
			input->mode = cpubridge::eWifiMode::hotSpot;
			break;

		case 1:
			input->mode = cpubridge::eWifiMode::connectTo;
			break;
		}
	}
	else if (strcasecmp((const char*)fieldName, "SSID") == 0)
	{
		rhea::string::utf8::spf (input->ssid, sizeof(input->ssid), "%s", fieldValue);
	}

	else if (strcasecmp((const char*)fieldName, "pwd") == 0)
	{
		rhea::string::utf8::spf (input->pwd, sizeof(input->pwd), "%s", fieldValue);
		return false;
	}
	return true;
}


//***********************************************************
void CmdHandler_ajaxReqWiFiSetMode::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	memset (&data, 0, sizeof(data));

	if (rhea::json::parse(params, ajaxReqWiFiSetMode_jsonTrapFunction, &data))
	{
		switch (data.mode)
		{
		default:
			cpubridge::ask_WIFI_SET_MODE_HOTSPOT (from, getHandlerID());
			break;

		case cpubridge::eWifiMode::connectTo:
			cpubridge::ask_WIFI_SET_MODE_CONNECTTO (from, getHandlerID(), data.ssid, data.pwd);
			break;
		}
	}		
}

//***********************************************************
void CmdHandler_ajaxReqWiFiSetMode::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char resp[8];
	sprintf_s(resp, sizeof(resp), "OK");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
