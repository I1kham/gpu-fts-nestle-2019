#include "UserCommand_upload.h"

//**********************************************************
const char*	UserCommand_upload::getExplain() const
{ 
	return "upload [filename]\nupload the [filename] to the SMU.\nThe SMU will store the file in the temp folder";
}

//**********************************************************
void UserCommand_upload::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::utf8::Iter src, temp;
	src.setup((const u8*)command);

	const rhea::UTF8Char cSpace(" ");
	if (!rhea::string::utf8::advanceUntil(src, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [filename]\n");
		return;
	}
	rhea::string::utf8::toNextValidChar(src);

	if (!rhea::string::utf8::extractValue(src, &temp, &cSpace, 1))
	{
		log->log("syntax error, expecting parameter [filename]\n");
		return;
	}

	u8 param1[512];
	temp.copyAllStr(param1, sizeof(param1));

	u8 filenameOnly[128];
	rhea::fs::extractFileNameWithExt(param1, filenameOnly, sizeof(filenameOnly));

	char uploadCommand[128];
	sprintf_s(uploadCommand, sizeof(uploadCommand), "store:%s", filenameOnly);



	rhea::app::FileTransfer::Handle handle;
	log->incIndent();

	if (ftransf->startFileUpload(ch, proto, rhea::getTimeNowMSec(), (const char*)param1, uploadCommand, &handle))
		log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
	else
		log->log("file transfer FAILED to start\n");
	
	log->decIndent();
}