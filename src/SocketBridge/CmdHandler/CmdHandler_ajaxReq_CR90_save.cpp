#include "CmdHandler_ajaxReq_CR90_save.h"
#include "../SocketBridge.h"
#include "../CR90File.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	u16		index;
	u16		value;
};

//***********************************************************
bool CmdHandler_ajaxReq_CR90_save_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "index") == 0)
	{
		input->index = static_cast<u16>(rhea::string::utf8::toU32(fieldValue));
	}
	if (strcasecmp((const char*)fieldName, "value") == 0)
	{
		input->value = static_cast<u16>(rhea::string::utf8::toU32(fieldValue));
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReq_CR90_save::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	memset (&data, 0, sizeof(data));
	if (rhea::json::parse(params, CmdHandler_ajaxReq_CR90_save_jsonTrapFunction, &data))
	{
		CR90File cr90;
		cr90.load();
		cr90.setValue (data.index, data.value);
		cr90.save();
	}

	const u8 answer[2] = { 'O', 'K'};
	server->sendAjaxAnwer(hClient, ajaxRequestID, answer, 2);
}
