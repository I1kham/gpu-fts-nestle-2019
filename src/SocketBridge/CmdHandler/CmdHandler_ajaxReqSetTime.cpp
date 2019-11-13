#include "CmdHandler_ajaxReqSetTime.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		h;
	u8		m;
	u8		s;
};

//***********************************************************
bool ajaxReqSetTime_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "h") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h <= 23)
			input->h = (u8)h;
		else
			input->h = 0;
	}
	else if (strcasecmp(fieldName, "m") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h <= 59)
			input->m = (u8)h;
		else
			input->m = 0;
	}
	else if (strcasecmp(fieldName, "s") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h <= 59)
			input->s = (u8)h;
		else
			input->s = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqSetTime::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqSetTime_jsonTrapFunction, &data))
		cpubridge::ask_CPU_SET_TIME(from, getHandlerID(), data.h, data.m, data.s);
}

//***********************************************************
void CmdHandler_ajaxReqSetTime::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 hh = 0, mm = 0, ss = 0;
	cpubridge::translateNotify_SET_TIME(msgFromCPUBridge, &hh, &mm, &ss);

	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
