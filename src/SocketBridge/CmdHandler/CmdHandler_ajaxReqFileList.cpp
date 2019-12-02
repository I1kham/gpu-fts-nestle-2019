#include "CmdHandler_ajaxReqFileList.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	char path[512];
	char jolly[128];
};

//***********************************************************
bool ajaxReqFileList_jsonTrapFunction(const char *fieldName, const char *fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (strcasecmp(fieldName, "path") == 0)
	{
		strncpy (input->path, fieldValue, sizeof(input->path)-1);
		return true;
	}
	else if (strcasecmp(fieldName, "jolly") == 0)
	{
		strncpy(input->jolly, fieldValue, sizeof(input->jolly) - 1);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFileList::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params)
{
	u16 handle = 0;
	sInput data;
	if (!rhea::json::parse(params, ajaxReqFileList_jsonTrapFunction, &data))
		return;

	rhea::Allocator *localAllocator = rhea::memory_getScrapAllocator();
	u32 sizeForFolderStr = 0;
	u32 sizeForFileStr = 0;
	char *fileList = NULL;
	char *folderList = NULL;
	OSFileFind h;
	if (rhea::fs::findFirst(&h, data.path, data.jolly))
	{
		do
		{
			const char *fname = rhea::fs::findGetFileName(h);
			if (rhea::fs::findIsDirectory(h))
			{
				if (fname[0] != '.')
					sizeForFolderStr += strlen(fname) + 1;
			}
			else
				sizeForFileStr += strlen(fname) + 1;
		} while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);

		if (sizeForFolderStr)
			folderList = (char*)RHEAALLOC(localAllocator, sizeForFolderStr);
		if (sizeForFileStr)
			fileList = (char*)RHEAALLOC(localAllocator, sizeForFileStr);

		rhea::fs::findFirst(&h, data.path, data.jolly);
		do
		{
			const char *fname = rhea::fs::findGetFileName(h);
			if (rhea::fs::findIsDirectory(h))
			{
				if (fname[0] != '.')
					sizeForFolderStr += strlen(fname) + 1;
			}
			else
				sizeForFileStr += strlen(fname) + 1;
		} while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

	char resp[64];
	sprintf(resp, "{\"path\":\"%s\", \"folderList\":[],\"fileList\":[]}", data.path);
	server->sendAjaxAnwer (hClient, ajaxRequestID, resp, (u16)strlen(resp));
}