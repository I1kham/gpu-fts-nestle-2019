#include "CmdHandler_ajaxReqDBCloseByPath.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	char path[256];
};

//***********************************************************
bool ajaxReqDBCloseByPath_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "path") == 0)
	{
		strncpy (input->path, fieldValue, sizeof(input->path)-1);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqDBCloseByPath::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqDBCloseByPath_jsonTrapFunction, &data))
		server->DB_closeByPath(data.path);

	char resp[8];
	sprintf(resp, "OK");
	server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}