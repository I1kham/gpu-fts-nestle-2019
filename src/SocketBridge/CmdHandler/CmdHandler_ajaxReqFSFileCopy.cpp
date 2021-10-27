#include "CmdHandler_ajaxReqFSFileCopy.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 pSRC[512];
	u8 fSRC[128];
	u8 pDST[512];
	u8 fDST[128];
};

//***********************************************************
bool ajaxReqFSFileCopy_jsonTrapFunction(const u8 *fieldName, const u8 *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "pSRC") == 0)
	{
		rhea::fs::sanitizePath(fieldValue, input->pSRC, sizeof(input->pSRC) - 1);
		return true;
	}
	else if (strcasecmp((const char*)fieldName, "fSRC") == 0)
	{
		rhea::string::utf8::copyStr (input->fSRC, sizeof(input->fSRC) - 1, fieldValue);
		return true;
	}
	else if (strcasecmp((const char*)fieldName, "pDST") == 0)
	{
		rhea::fs::sanitizePath(fieldValue, input->pDST, sizeof(input->pDST) - 1);
		return true;
	}
	else if (strcasecmp((const char*)fieldName, "fDST") == 0)
	{
		rhea::string::utf8::copyStr (input->fDST, sizeof(input->fDST) - 1, fieldValue);
		return false;
	}
	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFSFileCopy::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqFSFileCopy_jsonTrapFunction, &data))
		return;

	u8 src[1024];
	u8 dst[1024];
	sprintf_s((char*)src, sizeof(src), "%s/%s", data.pSRC, data.fSRC);
	sprintf_s((char*)dst, sizeof(dst), "%s/%s", data.pDST, data.fDST);

	char resp[8];
	if (rhea::fs::fileCopy (src, dst))
		sprintf(resp, "OK");
	else
		sprintf(resp, "KO");

	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)resp, (u16)strlen(resp));
}
