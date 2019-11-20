#include "CmdHandler_ajaxReqTestSelection.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		selNum;
	u8		deviceID;
};

//***********************************************************
bool ajaxReqTestSelection_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "s") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		if (h>=1 && h <= 64)
			input->selNum = (u8)h;
		else
			input->selNum = 0;
	}
	else if (strcasecmp(fieldName, "d") == 0)
	{
		const u32 h = rhea::string::convert::toU32(fieldValue);
		input->deviceID = (u8)h;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReqTestSelection::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqTestSelection_jsonTrapFunction, &data))
	{
		if (data.selNum != 0)
			cpubridge::ask_CPU_TEST_SELECTION(from, getHandlerID(), data.selNum, (cpubridge::eCPUProgrammingCommand_testSelectionDevice)data.deviceID);
	}
}

//***********************************************************
void CmdHandler_ajaxReqTestSelection::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 selNum = 0;
	cpubridge::eCPUProgrammingCommand_testSelectionDevice devID = cpubridge::eCPUProgrammingCommand_testSelectionDevice_unknown;
	cpubridge::translateNotify_CPU_TEST_SELECTION(msgFromCPUBridge, &selNum, &devID);

	char text[4] = { 'O', 'K', 0, 0 };
	if (selNum == 0xFF || devID == cpubridge::eCPUProgrammingCommand_testSelectionDevice_unknown)
	{
		text[0] = 'K';
		text[1] = 'O';
	}
    server->sendAjaxAnwer (hClient, ajaxRequestID, text, (u16)strlen(text));
}
