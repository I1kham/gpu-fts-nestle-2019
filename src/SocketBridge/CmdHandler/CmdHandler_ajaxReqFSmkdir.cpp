#include "CmdHandler_ajaxReqFSmkdir.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 path[1024];
};

//***********************************************************
bool ajaxReqFSFSmkdir_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "path") == 0)
	{
		rhea::fs::sanitizePath(fieldValue, input->path, sizeof(input->path) - 1);
		return false;
	}
	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFSmkdir::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqFSFSmkdir_jsonTrapFunction, &data))
		return;

	char resp[8];
	rhea::fs::sanitizePathInPlace (data.path);
	if (rhea::fs::folderCreate (data.path))
		sprintf(resp, "OK");
	else
		sprintf(resp, "KO");

	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
