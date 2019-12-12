#include "TaskCopyFolderToFolder.h"




//*********************************************************************
void TaskCopyFolderToFolder::run(socketbridge::TaskStatus *status, const char *params)
{
	char src[512];
	char dst[512];
	memset (dst, 0, sizeof(dst));

	strcpy_s(src, sizeof(src), params);
	for (u32 i = 0; i < strlen(src); i++)
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