#include "CmdHandler_ajaxReqFSDriveList.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


//***********************************************************
u8* CmdHandler_ajaxReqFSDriveList::reallocString(rhea::Allocator *allocator, u8 *cur, u32 curSize, u32 newSize) const
{
	u8 *s = (u8*)RHEAALLOC(allocator, newSize);
	memcpy(s, cur, curSize);
	RHEAFREE(allocator, cur);
	return s;
}


//***********************************************************
void CmdHandler_ajaxReqFSDriveList::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params UNUSED_PARAM)
{
	rhea::Allocator *localAllocator = rhea::getScrapAllocator();

	u32 drivePathMaxSize = 256;
	u32 drivePathCurSize = 0;
	u8 *drivePath = (u8*)RHEAALLOC(localAllocator, drivePathMaxSize);
	drivePath[0] = 0;

	u32 driveLabelMaxSize = 2048;
	u32 driveLabelCurSize = 0;
	u8 *driveLabel = (u8*)RHEAALLOC(localAllocator, driveLabelMaxSize);
	driveLabel[0] = 0;

	OSDriveEnumerator h;
	rheaFindHardDriveResult s;
	if (rhea::fs::findFirstHardDrive(&h, &s))
	{
		do
		{
			u32 n = rhea::string::utf8::lengthInBytes(s.utf8_drivePath);
			if (s.utf8_drivePath[n - 1] == '\\') 
			{
				n--;
				s.utf8_drivePath[n] = 0;
			}

			n+=3;
			if (drivePathCurSize + n >= drivePathMaxSize)
			{
				drivePath = reallocString(localAllocator, drivePath, drivePathMaxSize, drivePathMaxSize + 1024);
				drivePathMaxSize += 1024;
			}
			
			rhea::string::utf8::concatStr (drivePath, drivePathMaxSize, "\"");
			rhea::string::utf8::concatStr (drivePath, drivePathMaxSize, s.utf8_drivePath);
			rhea::string::utf8::concatStr (drivePath, drivePathMaxSize, "\",");
			drivePathCurSize += n;


			n = rhea::string::utf8::lengthInBytes(s.utf8_driveLabel) + 3;
			if (driveLabelCurSize + n >= driveLabelMaxSize)
			{
				driveLabel = reallocString(localAllocator, driveLabel, driveLabelMaxSize, driveLabelMaxSize + 1024);
				driveLabelMaxSize += 1024;
			}
			rhea::string::utf8::concatStr(driveLabel, driveLabelMaxSize, "\"");
			rhea::string::utf8::concatStr(driveLabel, driveLabelMaxSize, s.utf8_driveLabel);
			rhea::string::utf8::concatStr(driveLabel, driveLabelMaxSize, "\",");
			driveLabelCurSize += n;

		} while (rhea::fs::findNextHardDrive(h, &s));
		rhea::fs::findCloseHardDrive(h);

		if (drivePath[drivePathCurSize - 1] == ',')
		{
			--drivePathCurSize;
			drivePath[drivePathCurSize] = 0;
		}

		if (driveLabel[driveLabelCurSize - 1] == ',')
		{
			--driveLabelCurSize;
			driveLabel[driveLabelCurSize] = 0;
		}
	}


	u8 desktopPath[256];
	if (!rhea::fs::getDestkopPath(desktopPath, sizeof(desktopPath)))
		desktopPath[0] = 0;

	
    u8 *resp = (u8*)RHEAALLOC(localAllocator, 96 + drivePathCurSize + driveLabelCurSize +rhea::string::utf8::lengthInBytes(desktopPath));
	sprintf((char*)resp, "{\"drivePath\":[%s],\"driveLabel\":[%s],\"desktop\":\"%s\"}", drivePath, driveLabel, desktopPath);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)rhea::string::utf8::lengthInBytes(resp));

    RHEAFREE(localAllocator, drivePath);
	RHEAFREE(localAllocator, driveLabel);
	RHEAFREE(localAllocator, resp);
}
