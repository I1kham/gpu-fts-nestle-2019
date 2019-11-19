#include "CmdHandler_ajaxReqSetDecounter.h"
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
bool ajaxReqResetDecounter_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "d") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h > 0 && h < 0xff)
			input->d = (u8)h;
		else
			input->d = 0xff;
	}
	if (strcasecmp(fieldName, "v") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xffff)
			input->v = (u16)h;
		else
			input->v = 0xff;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqSetDecounter::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqResetDecounter_jsonTrapFunction, &data))
		cpubridge::ask_CPU_SET_DECOUNTER(from, getHandlerID(), (cpubridge::eCPUProgrammingCommand_decounter)data.d, data.v);
}

//***********************************************************
void CmdHandler_ajaxReqSetDecounter::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProgrammingCommand_decounter which = cpubridge::eCPUProgrammingCommand_decounter_unknown;
	u16 valore = 0;
	cpubridge::translateNotify_CPU_DECOUNTER_SET (msgFromCPUBridge, &which, &valore);


	char text[4] = { 'O', 'K', 0, 0 };

	if (which == cpubridge::eCPUProgrammingCommand_decounter_error)
	{
		text[0] = 'K';
		text[1] = 'O';
	}
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
