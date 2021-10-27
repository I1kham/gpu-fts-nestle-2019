#include "CmdHandler_ajaxReq_P0x04_SetDecounter.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		d;
	u16		v;
};

//***********************************************************
bool ajaxReqResetDecounter_jsonTrapFunction(const u8 *fieldName, const u8 *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "d") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h > 0 && h < 0xff)
			input->d = (u8)h;
		else
			input->d = 0xff;
	}
	if (strcasecmp((const char*)fieldName, "v") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
        if (h <= 0xffff)
			input->v = (u16)h;
		else
			input->v = 0xff;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x04_SetDecounter::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqResetDecounter_jsonTrapFunction, &data))
		cpubridge::ask_CPU_SET_DECOUNTER(from, getHandlerID(), (cpubridge::eCPUProg_decounter)data.d, data.v);
}

//***********************************************************
void CmdHandler_ajaxReq_P0x04_SetDecounter::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProg_decounter which = cpubridge::eCPUProg_decounter::unknown;
	u16 valore = 0;
	cpubridge::translateNotify_CPU_DECOUNTER_SET (msgFromCPUBridge, &which, &valore);


	char text[4] = { 'O', 'K', 0, 0 };

	if (which == cpubridge::eCPUProg_decounter::error)
	{
		text[0] = 'K';
		text[1] = 'O';
	}
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
