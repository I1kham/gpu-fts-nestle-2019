#include "CmdHandler_ajaxReq_P0x31_askMsgFromLangTable.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u8		tabID;
	u8		rowNum;
	u8		langNum;
};

//***********************************************************
bool CmdHandler_ajaxReq_P0x31_askMsgFromLangTable_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "tabID") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h >= 0 && h <= 0xff)
			input->tabID = (u8)h;
		else
			input->tabID = 0xff;
	}
	else if (strcasecmp((const char*)fieldName, "rowNum") == 0)
	{
		const u32 h = rhea::string::utf8::toU32(fieldValue);
		if (h >= 0 && h <= 0xff)
			input->rowNum = (u8)h;
		else
			input->rowNum = 0xff;
	}
	else if (strcasecmp((const char*)fieldName, "lang") == 0)
	{
		input->langNum = static_cast<u8>(rhea::string::utf8::toU32(fieldValue));
		return false;
	}
	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_P0x31_askMsgFromLangTable::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params)
{
	sInput data;
	memset (&data, 0, sizeof(data));
	if (rhea::json::parse(params, CmdHandler_ajaxReq_P0x31_askMsgFromLangTable_jsonTrapFunction, &data))
		cpubridge::ask_MSG_FROM_LANGUAGE_TABLE(from, getHandlerID(), data.tabID, data.rowNum, data.langNum);
}

//***********************************************************
void CmdHandler_ajaxReq_P0x31_askMsgFromLangTable::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u8 tabID;
	u8 rowNum;
	u8 utf8Msg[256];
	memset (utf8Msg, 0, sizeof(utf8Msg));
	cpubridge::translateNotify_MSG_FROM_LANGUAGE_TABLE (msgFromCPUBridge, &tabID, &rowNum, utf8Msg, sizeof(utf8Msg));


	u8 answer[512];
	rhea::string::utf8::spf (answer, sizeof(answer), "{\"tabID\":\"%d\",\"rowNum\":\"%d\",\"msg\":\"%s\"}", tabID, rowNum, utf8Msg);
    server->sendAjaxAnwer (hClient, ajaxRequestID, answer, rhea::string::utf8::lengthInBytes(answer));
}
