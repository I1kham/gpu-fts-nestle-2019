#include "TaskCopyFolderToFolder.h"




//*********************************************************************
void TaskCopyFolderToFolder::run(socketbridge::TaskStatus *status, const u8 *params)
{
	u8 src[512];
	u8 dst[512];
	memset (dst, 0, sizeof(dst));

	rhea::string::utf8::copyStr (src, sizeof(src), params);
	const u32 len = rhea::string::utf8::lengthInBytes(src);
	for (u32 i = 0; i < len; i++)
	{
		if (src[i] == '§')
		{
			src[i] = 0x00;
			rhea::fs::sanitizePath(&src[i + 1], dst, sizeof(dst));
			rhea::fs::sanitizePathInPlace(src);
			break;
		}
	}

	if (dst[0] == 0x00)
	{
		status->setMessage("Invalid path");
		return;
	}

	//creo il folder temp
	if (!rhea::fs::folderCreate(dst))
	{
		status->setMessage("Error creating %s", dst);
		return;
	}

	//copio
	status->setMessage("Copying files...");
	if (!rhea::fs::folderCopy (src, dst))
	{
		status->setMessage("Error copying files");
		return;
	}


	status->setMessage("OK");
}