#include "CmdHandler_ajaxReq_P0x0A_SetDate.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u16		y;
	u8		m;
	u8		d;
};

//***********************************************************
bool ajaxReqSetDate_jsonTrapFunction(const u8* fieldName, const u8* fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "y") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h >= 2000 && h <=2099)
			input->y = (u16)h;
		else
			input->y = 2000;
	}
	else if (strcasecmp((const char*)fieldName, "m") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h >= 1 && h<=12)
			input->m = (u8)h;
		else
			input->m = 1;
	}
	else if (strcasecmp((const char*)fieldName, "d") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h >= 1 && h<=31)
			input->d = (u8)h;
		else
			input->d = 1;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x0A_SetDate::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqSetDate_jsonTrapFunction, &data))
		cpubridge::ask_CPU_SET_DATE(from, getHandlerID(), data.y, data.m, data.d);
}

//***********************************************************
void CmdHandler_ajaxReq_P0x0A_SetDate::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 year = 0;
	u8 month = 0;
	u8 day = 0;
	cpubridge::translateNotify_SET_DATE(msgFromCPUBridge, &year, &month, &day);

	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
