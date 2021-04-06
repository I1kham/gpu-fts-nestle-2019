#include "CmdHandler_ajaxReq_setLastUsedLangForProgMenu.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"

using namespace socketbridge;

struct sInput
{
	char lang[4];
};

//***********************************************************
bool ajaxReqSetLastUsedLangForProgMenu_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp((const char*)fieldName, "lang") == 0)
	{
		input->lang[0] = fieldValue[0];
		input->lang[1] = fieldValue[1];
		input->lang[2] = 0;
		return false;
	}

	return true;
}


//***********************************************************
void CmdHandler_ajaxReq_setLastUsedLangForProgMenu::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (rhea::json::parse(params, ajaxReqSetLastUsedLangForProgMenu_jsonTrapFunction, &data))
	{
		//devo creare o sovrascrivere il file varie/prog/lastUsedLang.txt e riempirlo con il codice della lingua
		u8 s[512];

		sprintf_s((char*)s, sizeof(s), "%s/varie/prog", rhea::getPhysicalPathToAppFolder());
		rhea::fs::folderCreate (s);

		strcat_s ((char*)s, sizeof(s), "/lastUsedLang.txt");
		FILE *f = rhea::fs::fileOpenForWriteBinary(s);
		fwrite(data.lang, 2, 1, f);
		rhea::fs::fileClose(f);
	}


	server->sendAjaxAnwer(hClient, ajaxRequestID, (const u8*)"OK", 2);
}
