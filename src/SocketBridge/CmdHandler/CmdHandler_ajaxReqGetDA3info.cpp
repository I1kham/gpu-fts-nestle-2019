#include "CmdHandler_ajaxReqGetDA3info.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReqGetDA3info::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	u8			fullPath[256], fileName[128], datetime[32];
	u32			strLen;
	OSFileFind	h;

	u64			lastUpdate, fileSize;
	FILE* fd;

	strLen = sprintf_s((char*)fullPath, sizeof(fullPath), "%s/last_installed/da3", rhea::getPhysicalPathToAppFolder());
	rhea::fs::sanitizePathInPlace(fullPath, strLen);

	memset(fileName, 0, sizeof fileName);

	if (rhea::fs::findFirst(&h, fullPath, (const u8*)"*.da3"))
	{
		bool	exitLoop = false;

		do
		{
			if (!rhea::fs::findIsDirectory(h))
			{
				const u8* fName = rhea::fs::findGetFileName(h);
				if (fName[0] != '.')
				{
					sprintf_s((char*)fileName, sizeof fileName, "%s", fName);
					exitLoop = true;
				}
			}
		} while (false == exitLoop && rhea::fs::findNext(h));
		rhea::fs::findClose(h);
	}

	if (fileName[0] == 0x00)
	{
		strcpy_s((char*)fileName, sizeof(fileName), "???");
		strcpy_s((char*)datetime, sizeof(datetime), "???");
	}
	else
	{
		// caricamento della data
		memset(&datetime, 0, sizeof datetime);
		strcat_s((char*)fullPath, sizeof fullPath, "/dateUM.bin");
		fd = rhea::fs::fileOpenForReadText(fullPath);
		if (NULL != fd && sizeof(u64) <= (fileSize = rhea::fs::filesize(fd)))
		{
			rhea::DateTime dt;

            rhea::fs::fileRead (fd, &lastUpdate, sizeof(lastUpdate));
            rhea::fs::fileClose(fd);

			dt.setFromInternalRappresentation(lastUpdate);
			dt.formatAs_YYYYMMDDHHMMSS((char*)datetime, sizeof(datetime), ' ', '/', ':');
		}
		else
			strcpy_s((char*)datetime, sizeof datetime, "???");
	}

	sprintf_s((char *)fullPath, sizeof(fullPath),
				"{ \"filename\" : \"%s\", \"lastUpdate\" : \"%s\"}",
				fileName, 
				datetime);

	server->sendAjaxAnwer(hClient, ajaxRequestID, fullPath, (u16)rhea::string::utf8::lengthInBytes(fullPath));
}
