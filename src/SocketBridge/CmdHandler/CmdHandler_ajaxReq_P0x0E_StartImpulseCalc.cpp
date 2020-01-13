#include "CmdHandler_ajaxReq_P0x0E_StartImpulseCalc.h"
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
        if (h <= 0xff)
			input->m = (u8)h;
		else
			input->m = 0;
	}
	if (strcasecmp(fieldName, "v") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xffff)
			input->v = (u16)h;
		else
			input->v = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x0E_StartImpulseCalc::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqStartImpulseCalc_jsonTrapFunction, &data))
	{
		if (data.m == 12) data.m = 2;
		else data.m = 1;
		cpubridge::ask_CPU_CALCOLA_IMPULSI_GRUPPO(from, getHandlerID(), data.m, data.v);
	}
}

//***********************************************************
void CmdHandler_ajaxReq_P0x0E_StartImpulseCalc::onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)
{
	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
