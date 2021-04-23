#include "CmdHandler_ajaxReqTaskStatus.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u32 id;
};

//***********************************************************
bool ajaxReqTaskStatus_jsonTrapFunction(const u8* fieldName, const u8* fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "id") == 0)
	{
		input->id = rhea::string::utf8::toU32(fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqTaskStatus::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqTaskStatus_jsonTrapFunction, &data))
		return;

	char resp[256];
	TaskStatus::eStatus status;
	char taskMsg[256];
	if (!server->taskGetStatusAndMesssage(data.id, &status, taskMsg, sizeof(taskMsg)))
		sprintf_s(resp, sizeof(resp), "{\"status\":0,\"msg\":\"\"}");
	else
	{
		u8 reportedStatus = 1;
		if (status == TaskStatus::eStatus_finished)
			reportedStatus = 0;
		sprintf_s(resp, sizeof(resp), "{\"status\":%d,\"msg\":\"%s\"}", reportedStatus, taskMsg);
	}

	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
