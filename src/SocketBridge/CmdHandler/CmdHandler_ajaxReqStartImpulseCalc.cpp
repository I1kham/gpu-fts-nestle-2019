#include "CmdHandler_ajaxReqStartImpulseCalc.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		m;
	u16		v;
};

//***********************************************************
bool ajaxReqStartImpulseCalc_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "m") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h >= 0 && h <= 0xff)
			input->m = (u8)h;
		else
			input->m = 0;
	}
	if (strcasecmp(fieldName, "v") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h >= 0 && h <= 0xffff)
			input->v = (u16)h;
		else
			input->v = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqStartImpulseCalc::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqStartImpulseCalc_jsonTrapFunction, &data))
		cpubridge::ask_CPU_CALCOLA_IMPULSI_GRUPPO (from, getHandlerID(), data.m, data.v);
}

//***********************************************************
void CmdHandler_ajaxReqStartImpulseCalc::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
