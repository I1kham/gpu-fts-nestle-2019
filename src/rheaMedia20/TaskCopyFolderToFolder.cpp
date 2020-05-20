#include "TaskCopyFolderToFolder.h"




//*********************************************************************
void TaskCopyFolderToFolder::run(socketbridge::TaskStatus *status, const u8 *params)
{
	const rhea::UTF8Char SEP("§");
	rhea::string::utf8::Iter iter1;
	iter1.setup (params);

	if (!rhea::string::utf8::advanceUntil(iter1, &SEP, 1))
	{
		status->setMessage("Invalid path");
		return;
	}
	u8 src[512];
	u8 dst[512];
	iter1.copyStrFromXToCurrentPosition (0, src, sizeof(src), false);
	rhea::fs::sanitizePathInPlace (src);
	iter1.advanceOneChar();
	iter1.copyStrFromCurrentPositionToEnd (dst, sizeof(dst));
	rhea::fs::sanitizePathInPlace (dst);
	
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