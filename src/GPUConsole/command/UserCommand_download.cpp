#include "UserCommand_download.h"

//**********************************************************
const char*	UserCommand_download::getExplain() const
{ 
	return "download [what]\n[what] = audit[n] | da3[n] | test"; 
}

//**********************************************************
void UserCommand_download::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::utf8::Iter src, temp;
	src.setup((const u8*)command);

	const rhea::UTF8Char cSpace(" ");
	if (!rhea::string::utf8::advanceUntil(src, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [what]\n");
		return;
	}
	rhea::string::utf8::toNextValidChar(src);

	if (!rhea::string::utf8::extractValue(src, &temp, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [what]\n");
		return;
	}

	u8 what[64];
	temp.copyAllStr(what, sizeof(what));


	rhea::app::FileTransfer::Handle handle;
	log->incIndent();


	char downloadedFilePathAndName[512];
	downloadedFilePathAndName[0] = 0x00;
	if (rhea::string::utf8::areEqual(what, (const u8*)"test", true))
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/file_downloadata_da_smu", rhea::getPhysicalPathToWritableFolder());
	}
	else if (rhea::string::utf8::areEqualWithLen(what, (const u8*)"audit", true, 5))
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/audit.txt", rhea::getPhysicalPathToWritableFolder());
	}
	else if (rhea::string::utf8::areEqualWithLen(what, (const u8*)"da3", true, 3))
	{
		sprintf_s(downloadedFilePathAndName, sizeof(downloadedFilePathAndName), "%s/vmcDataFile.da3", rhea::getPhysicalPathToWritableFolder());
	}
	else
	{
		log->log("syntax error, invalid [what]\n");
	}




	if (downloadedFilePathAndName[0] != 0x00)
	{
		log->outText(false, true, true, "dst file is: %s\n", downloadedFilePathAndName);
		if (ftransf->startFileDownload(ch, proto, rhea::getTimeNowMSec(), (const char*)what, downloadedFilePathAndName, &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
	}
	
	log->decIndent();
}