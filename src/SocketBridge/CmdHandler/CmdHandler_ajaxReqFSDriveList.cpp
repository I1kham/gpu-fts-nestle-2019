#include "CmdHandler_ajaxReqFSDriveList.h"
#include "../SocketBridge.h"
#include "../../rheaCommonLib/rheaJSONParser.h"
#include "../../rheaDB/SQLInterface.h"


using namespace socketbridge;


//***********************************************************
char* CmdHandler_ajaxReqFSDriveList::reallocString(rhea::Allocator *allocator, char *cur, u32 curSize, u32 newSize) const
{
	char *s = (char*)RHEAALLOC(allocator, newSize);
	memcpy(s, cur, curSize);
	RHEAFREE(allocator, cur);
	return s;
}


//***********************************************************
void CmdHandler_ajaxReqFSDriveList::handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params UNUSED_PARAM)
{
	rhea::Allocator *localAllocator = rhea::memory_getScrapAllocator();

	u32 drivePathMaxSize = 256;
	u32 drivePathCurSize = 0;
	char *drivePath = (char*)RHEAALLOC(localAllocator, drivePathMaxSize);
	drivePath[0] = 0;

	u32 driveLabelMaxSize = 2048;
	u32 driveLabelCurSize = 0;
	char *driveLabel = (char*)RHEAALLOC(localAllocator, driveLabelMaxSize);
	driveLabel[0] = 0;

	OSDriveEnumerator h;
	rheaFindHardDriveResult s;
	if (rhea::fs::findFirstHardDrive(&h, &s))
	{
		do
		{
			u32 n = (u32)strlen(s.drivePath);
			if (s.drivePath[n - 1] == '\\') 
			{
				n--;
				s.drivePath[n] = 0;
			}

			n+=3;
			if (drivePathCurSize + n >= drivePathMaxSize)
			{
				drivePath = reallocString(localAllocator, drivePath, drivePathMaxSize, drivePathMaxSize + 1024);
				drivePathMaxSize += 1024;
			}
			
			strcat_s (drivePath, drivePathMaxSize, "\"");
			strcat_s (drivePath, drivePathMaxSize, s.drivePath);
			strcat_s (drivePath, drivePathMaxSize, "\",");
			drivePathCurSize += n;


			n = (u32)strlen(s.driveLabel) + 3;
			if (driveLabelCurSize + n >= driveLabelMaxSize)
			{
				driveLabel = reallocString(localAllocator, driveLabel, driveLabelMaxSize, driveLabelMaxSize + 1024);
				driveLabelMaxSize += 1024;
			}
			strcat_s(driveLabel, driveLabelMaxSize, "\"");
			strcat_s(driveLabel, driveLabelMaxSize, s.driveLabel);
			strcat_s(driveLabel, driveLabelMaxSize, "\",");
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

	
    char *resp = (char*)RHEAALLOC(localAllocator, 64 + drivePathCurSize + driveLabelCurSize);
    sprintf(resp, "{\"drivePath\":[%s],\"driveLabel\":[%s]}", drivePath, driveLabel);
	server->sendAjaxAnwer(hClient, ajaxRequestID, resp, (u16)strlen(resp));

    RHEAFREE(localAllocator, drivePath);
	RHEAFREE(localAllocator, driveLabel);
	RHEAFREE(localAllocator, resp);
}
