#include "UserCommand_startcleaning.h"

//**********************************************************
const char*	UserCommand_startcleaning::getExplain() const
{ 
	return "startcleaning [type] => type 1..4 = mixer 1..4; 5=milker; 8=sanitario; 160=rinsing";
}

//**********************************************************
void UserCommand_startcleaning::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [cleantType]\n");
		return;
	}
	rhea::string::parser::toNextValidChar(src);

	i32 cleantType = 0;
	if (!rhea::string::parser::extractInteger(src, &cleantType))
	{
		log->log("syntax error, expecting integer parameter [cleantType]\n");
		return;
	}

	rhea::app::ExecuteProgramCmd::askCleaning (ch, proto, (cpubridge::eCPUProgrammingCommand_cleaningType)cleantType);
	log->log("request sent\n");

}