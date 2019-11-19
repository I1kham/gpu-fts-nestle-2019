#include "CmdHandler_ajaxReqAttivazioneMotore.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		m;
	u8		d;
	u8		n;
	u8		p;
};

//***********************************************************
bool ajaxReqAttivazioneMotore_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "m") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xff)
			input->m = (u8)h;
		else
			input->m = 0xff;
	}
	if (strcasecmp(fieldName, "d") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xff)
			input->d = (u8)h;
		else
			input->d = 0;
	}
	if (strcasecmp(fieldName, "n") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xff)
			input->n = (u8)h;
		else
			input->n = 1;
	}
	if (strcasecmp(fieldName, "p") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
        if (h <= 0xff)
			input->p = (u8)h;
		else
			input->p = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqAttivazioneMotore::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqAttivazioneMotore_jsonTrapFunction, &data))
	{
		if (data.m >= 1 && data.m <= 12)
		{
			if (data.n < 1)
				data.n = 1;
			cpubridge::ask_CPU_ATTIVAZIONE_MOTORE(from, getHandlerID(), (cpubridge::eCPUProgrammingCommand_motor)data.m, data.d, data.n, data.p);
		}
	}
}

//***********************************************************
void CmdHandler_ajaxReqAttivazioneMotore::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	cpubridge::eCPUProgrammingCommand_motor motor = cpubridge::eCPUProgrammingCommand_motor_unknown;
	u8 motore = 0;
	u8 durata_dsec = 0;
	u8 numRipetizioni = 0;
	u8 pausa_dSec = 0;
	cpubridge::translateNotify_ATTIVAZIONE_MOTORE(msgFromCPUBridge, &motore, &durata_dsec, &numRipetizioni, &pausa_dSec);

	char text[4] = { 'O', 'K', 0, 0 };
	if (motore == 0xFF)
	{
		text[0] = 'K';
		text[1] = 'O';
	}
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
