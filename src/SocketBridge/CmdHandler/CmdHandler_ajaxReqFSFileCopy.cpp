#include "CmdHandler_ajaxReqFSFileCopy.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	char pSRC[512];
	char fSRC[128];
	char pDST[512];
	char fDST[128];
};

//***********************************************************
bool ajaxReqFSFileCopy_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "pSRC") == 0)
	{
		rhea::fs::sanitizePath(fieldValue, input->pSRC, sizeof(input->pSRC) - 1);
		return true;
	}
	else if (strcasecmp(fieldName, "fSRC") == 0)
	{
		strncpy(input->fSRC, fieldValue, sizeof(input->fSRC) - 1);
		return true;
	}
	else if (strcasecmp(fieldName, "pDST") == 0)
	{
		rhea::fs::sanitizePath(fieldValue, input->pDST, sizeof(input->pDST) - 1);
		return true;
	}
	else if (strcasecmp(fieldName, "fDST") == 0)
	{
		strncpy(input->fDST, fieldValue, sizeof(input->fDST) - 1);
		return false;
	}
	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFSFileCopy::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqFSFileCopy_jsonTrapFunction, &data))
		return;

	char src[1024];
	char dst[1024];
	sprintf_s(src, sizeof(src), "%s/%s", data.pSRC, data.fSRC);
	sprintf_s(dst, sizeof(dst), "%s/%s", data.pDST, data.fDST);

	char resp[8];
	if (rhea::fs::fileCopy (src, dst))
		sprintf(resp, "OK");
	else
		sprintf(resp, "KO");

	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));
}
