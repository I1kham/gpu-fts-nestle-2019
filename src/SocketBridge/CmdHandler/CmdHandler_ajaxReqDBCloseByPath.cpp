#include "CmdHandler_ajaxReqDBCloseByPath.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 path[256];
};

//***********************************************************
bool ajaxReqDBCloseByPath_jsonTrapFunction(const u8* fieldName, const u8* fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "path") == 0)
	{
		rhea::string::utf8::copyStr (input->path, sizeof(input->path) - 1, fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBCloseByPath::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqDBCloseByPath_jsonTrapFunction, &data))
		server->DB_closeByPath(data.path);

	char resp[8];
	sprintf(resp, "OK");
	server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}