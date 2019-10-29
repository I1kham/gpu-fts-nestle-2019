#include "UserCommand_startsel.h"

//**********************************************************
const char*	UserCommand_startsel::getExplain() const
{ 
	return "startsel [selnum] => selum >= 1 && selnnum <=48";
}

//**********************************************************
void UserCommand_startsel::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [selnum]\n");
		return;
	}
	rhea::string::parser::toNextValidChar(src);

	i32 selNum = 0;
	if (!rhea::string::parser::extractInteger(src, &selNum))
	{
		log->log("syntax error, expecting integer parameter [selnum]\n");
		return;
	}

	if (selNum >= 1 && selNum <= 48)
	{
		rhea::app::ExecuteSelection::ask(ch, proto, (u8)selNum);
		log->log("request sent\n");
	}
	else
		log->log("syntax error, invalid [selnum]. Good values are between 1 and 48\n");
}