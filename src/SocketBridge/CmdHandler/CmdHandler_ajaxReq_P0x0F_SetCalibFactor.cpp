#include "CmdHandler_ajaxReq_P0x0F_SetCalibFactor.h"
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
bool ajaxReqSetCalibFactor_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "m") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h > 0 && h < 0xff)
			input->m = (u8)h;
		else
			input->m = 0xff;
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
void CmdHandler_ajaxReq_P0x0F_SetCalibFactor::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqSetCalibFactor_jsonTrapFunction, &data))
	{
		if (data.m >= 1 && data.m < 20)
			cpubridge::ask_CPU_SET_FATTORE_CALIB_MOTORE(from, getHandlerID(), (cpubridge::eCPUProg_motor)data.m, data.v);
	}
}

//***********************************************************
void CmdHandler_ajaxReq_P0x0F_SetCalibFactor::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProg_motor motor = cpubridge::eCPUProg_motor::unknown;
	u16 valore = 0;
	cpubridge::translateNotify_SET_FATTORE_CALIB_MOTORE (msgFromCPUBridge, &motor, &valore);

	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
