#include "CmdHandler_ajaxReqLTEModemEnable.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		enable;
};

//***********************************************************
bool ajaxReqLTEModemEnable_jsonTrapFunction(const u8 *const fieldName, const u8 *const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "e") == 0)
	{
		if (rhea::string::utf8::toU32(fieldValue) > 0)
			input->enable = 1;
		else
			input->enable = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqLTEModemEnable::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	memset (&data, 0, sizeof(data));

	if (rhea::json::parse(params, ajaxReqLTEModemEnable_jsonTrapFunction, &data))
		cpubridge::ask_MODEM_LTE_ENABLE(from, getHandlerID(), data.enable ? true : false);
}

//***********************************************************
void CmdHandler_ajaxReqLTEModemEnable::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char resp[8];
	sprintf_s(resp, sizeof(resp), "OK");
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
