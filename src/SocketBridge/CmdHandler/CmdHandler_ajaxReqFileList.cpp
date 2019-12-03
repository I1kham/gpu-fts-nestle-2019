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
		rhea::fs::sanitizePath(fieldValue, input->path, sizeof(input->path) - 1);
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
					sizeForFolderStr += strlen(fname) + 2;
			}
			else
				sizeForFileStr += strlen(fname) + 2;
		} while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);

		if (sizeForFolderStr)
        {
            sizeForFolderStr+=2;
			folderList = (char*)RHEAALLOC(localAllocator, sizeForFolderStr);
            memset(folderList,0,sizeForFolderStr);
        }
		if (sizeForFileStr)
        {
            sizeForFileStr+=2;
			fileList = (char*)RHEAALLOC(localAllocator, sizeForFileStr);
            memset(fileList,0,sizeForFileStr);
        }

		const char SEP[4] = { (char)0xc2, (char)0xa7, 0, 0 };

		rhea::fs::findFirst(&h, data.path, data.jolly);
		do
		{
			const char *fname = rhea::fs::findGetFileName(h);
			if (rhea::fs::findIsDirectory(h))
			{
				if (fname[0] != '.')
                {
                    if (folderList[0] != 0x00) strcat (folderList, SEP);
                    strcat (folderList, fname);
                }
			}
			else
            {
                if (fileList[0] != 0x00) strcat (fileList, SEP);
                strcat (fileList, fname);
            }
        } while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

    if (0 == sizeForFolderStr)
    {
        sizeForFolderStr=2;
        folderList = (char*)RHEAALLOC(localAllocator, sizeForFolderStr);
        folderList[0] = 0x00;
        folderList[1] = 0x00;
    }

    if (0 == sizeForFileStr)
    {
        sizeForFileStr=2;
        fileList = (char*)RHEAALLOC(localAllocator, sizeForFileStr);
        fileList[0] = 0x00;
        fileList[1] = 0x00;
    }

    char *resp = (char*)RHEAALLOC(localAllocator, 64+strlen(data.path) +sizeForFolderStr +sizeForFileStr);
    sprintf(resp, "{\"path\":\"%s\",\"folderList\":\"%s\",\"fileList\":\"%s\"}", data.path, folderList, fileList);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));

    RHEAFREE(localAllocator, folderList);
    RHEAFREE(localAllocator, fileList);
    RHEAFREE(localAllocator, resp);
}
