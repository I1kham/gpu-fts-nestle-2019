#include "CmdHandler_ajaxReq_browserNotifyURLChange.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	char url[256];
};

//***********************************************************
bool CmdHandler_ajaxReq_browserNotifyURLChange_jsonTrapFunction(const u8 *fieldName, const u8 *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "url") == 0)
	{
		sprintf_s (input->url, sizeof(input->url), "%s", fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReq_browserNotifyURLChange::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	memset (&data, 0, sizeof(data));
	if (rhea::json::parse(params, CmdHandler_ajaxReq_browserNotifyURLChange_jsonTrapFunction, &data))
		cpubridge::ask_CPU_BROWSER_URL_CHANGE(from, getHandlerID(), data.url);
}

//***********************************************************
void CmdHandler_ajaxReq_browserNotifyURLChange::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
