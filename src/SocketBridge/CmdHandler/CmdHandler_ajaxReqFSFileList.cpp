#include "CmdHandler_ajaxReqFSFileList.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


struct sInput
{
	u8 path[512];
	u8 jolly[128];
};

//***********************************************************
bool ajaxReqFSFileList_jsonTrapFunction(const u8* const fieldName, const u8* const fieldValue, void *userValue)
{
	sInput *input = (sInput*)userValue;

	if (rhea::string::utf8::areEqual (fieldName, (const u8*)"path", true))
	{
		rhea::fs::sanitizePath(fieldValue, input->path, sizeof(input->path) - 1);
		return true;
	}
	else if (rhea::string::utf8::areEqual (fieldName, (const u8*)"jolly", true))
	{
		rhea::string::utf8::copyStr (input->jolly, sizeof(input->jolly) - 1, fieldValue);
		return false;
	}

	return true;
}

//***********************************************************
void CmdHandler_ajaxReqFSFileList::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params)
{
	sInput data;
	if (!rhea::json::parse(params, ajaxReqFSFileList_jsonTrapFunction, &data))
		return;

	rhea::Allocator *localAllocator = rhea::getScrapAllocator();
	u32 sizeForFolderStr = 0;
	u32 sizeForFileStr = 0;
	u8 *fileList = NULL;
	u8 *folderList = NULL;
	char canGoUpOneFolder = '0';
	OSFileFind h;
	if (rhea::fs::findFirst(&h, data.path, data.jolly))
	{
		do
		{
			const u8 *fname = rhea::fs::findGetFileName(h);
			if (rhea::fs::findIsDirectory(h))
			{
				if (fname[0] != '.' && fname[0] != '$')
					sizeForFolderStr += rhea::string::utf8::lengthInBytes(fname) + 2;
				else
				{
					if (fname[1] == '.')
						canGoUpOneFolder = '1';
				}
			}
			else
				sizeForFileStr += rhea::string::utf8::lengthInBytes(fname) + 2;
		} while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);

		if (sizeForFolderStr)
        {
            sizeForFolderStr+=2;
			folderList = (u8*)RHEAALLOC(localAllocator, sizeForFolderStr);
            memset(folderList,0,sizeForFolderStr);
        }
		if (sizeForFileStr)
        {
            sizeForFileStr+=2;
			fileList = (u8*)RHEAALLOC(localAllocator, sizeForFileStr);
            memset(fileList,0,sizeForFileStr);
        }

		const u8 SEP[4] = { (u8)0xc2, (u8)0xa7, 0, 0 };

		rhea::fs::findFirst(&h, data.path, data.jolly);
		do
		{
			const u8 *fname = rhea::fs::findGetFileName(h);
			if (rhea::fs::findIsDirectory(h))
			{
				if (fname[0] != '.' && fname[0] != '$')
                {
                    if (folderList[0] != 0x00) rhea::string::utf8::concatStr (folderList, sizeForFolderStr, SEP);
					rhea::string::utf8::concatStr (folderList, sizeForFolderStr, fname);
                }
			}
			else
            {
                if (fileList[0] != 0x00) rhea::string::utf8::concatStr (fileList, sizeForFileStr, SEP);
                rhea::string::utf8::concatStr (fileList, sizeForFileStr, fname);
            }
        } while (rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

    if (0 == sizeForFolderStr)
    {
        sizeForFolderStr=2;
        folderList = (u8*)RHEAALLOC(localAllocator, sizeForFolderStr);
        folderList[0] = 0x00;
        folderList[1] = 0x00;
    }

    if (0 == sizeForFileStr)
    {
        sizeForFileStr=2;
        fileList = (u8*)RHEAALLOC(localAllocator, sizeForFileStr);
        fileList[0] = 0x00;
        fileList[1] = 0x00;
    }


    u8 *resp = (u8*)RHEAALLOC(localAllocator, 64 + rhea::string::utf8::lengthInBytes(data.path) +sizeForFolderStr +sizeForFileStr);
    sprintf((char*)resp, "{\"path\":\"%s\",\"up\":%c, \"folderList\":\"%s\",\"fileList\":\"%s\"}", data.path, canGoUpOneFolder, folderList, fileList);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)rhea::string::utf8::lengthInBytes(resp));

    RHEAFREE(localAllocator, folderList);
    RHEAFREE(localAllocator, fileList);
    RHEAFREE(localAllocator, resp);
}
