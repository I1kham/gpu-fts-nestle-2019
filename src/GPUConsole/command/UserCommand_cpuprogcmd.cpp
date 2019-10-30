#include "UserCommand_cpuprogcmd.h"

//**********************************************************
const char*	UserCommand_cpuprogcmd::getExplain() const
{ 
	return "cpuprogcmd [cmd] | [param1] | [param1] | [param1] | [param1] => [cmd]:1=enterprog, 2=cleaning...  [param1..4]=optional, depending on [cmd]\n";
}

//**********************************************************
void UserCommand_cpuprogcmd::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [cmd]\n");
		return;
	}
	rhea::string::parser::toNextValidChar(src);

	i32 cmd = 0;
	if (!rhea::string::parser::extractInteger(src, &cmd))
	{
		log->log("syntax error, expecting integer parameter [cmd]\n");
		return;
	}


	//param1..4 sono opzionali
	u8 params[4] = { 0,0,0,0 };
	u8 ct = 0;
	while (ct < 4)
	{
		rhea::string::parser::toNextValidChar(src);
		i32 par;
		if (!rhea::string::parser::extractInteger(src, &par))
			break;
		params[ct++] = (u8)par;
	}

	rhea::app::ExecuteProgramCmd::ask(ch, proto, (cpubridge::eCPUProgrammingCommand)cmd, params[0], params[1], params[2], params[3]);
	log->log("request sent\n");

}