#include "CmdHandler_ajaxReqSetCalibFactor.h"
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
bool ajaxReqSetCalibFactor_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "m") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h > 0 && h < 0xff)
			input->m = (u8)h;
		else
			input->m = 0xff;
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
void CmdHandler_ajaxReqSetCalibFactor::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqSetCalibFactor_jsonTrapFunction, &data))
	{
		if (data.m >= 1 && data.m <= 12)
			cpubridge::ask_CPU_SET_FATTORE_CALIB_MOTORE(from, getHandlerID(), (cpubridge::eCPUProgrammingCommand_motor)data.m, data.v);
	}
}

//***********************************************************
void CmdHandler_ajaxReqSetCalibFactor::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProgrammingCommand_motor motor = cpubridge::eCPUProgrammingCommand_motor_unknown;
	u16 valore = 0;
	cpubridge::translateNotify_SET_FATTORE_CALIB_MOTORE (msgFromCPUBridge, &motor, &valore);

	char text[4] = { 'O', 'K', 0, 0 };
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
