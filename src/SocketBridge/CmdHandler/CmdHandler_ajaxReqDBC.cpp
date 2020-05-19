#include "CmdHandler_ajaxReqDBC.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 path[256];
};

//***********************************************************
bool ajaxReqDBC_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
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
void CmdHandler_ajaxReqDBC::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	u16 handle = 0;
	sInput data;
	if (rhea::json::parse(params, ajaxReqDBC_jsonTrapFunction, &data))
		handle = server->DB_getOrCreateHandle(data.path);

	char resp[64];
	sprintf(resp, "{\"handle\":%d}", handle);
	server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}