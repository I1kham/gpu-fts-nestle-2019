#include "UserCommand_setAperturaVgrind.h"

//**********************************************************
const char*	UserCommand_setAperturaVgrind::getExplain() const
{ 
	return "setaperturavgrind [macina_1o2] [target]";
}

//**********************************************************
void UserCommand_setAperturaVgrind::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	rhea::string::parser::Iter src, temp;
	src.setup(command);

	if (!rhea::string::parser::advanceUntil(src, " ", 1))
	{
		log->log("syntax error, expecting parameter [macina_1o2]\n");
		return;
	}
	

	i32 macina_1o2 = 0;
	rhea::string::parser::toNextValidChar(src);
	if (!rhea::string::parser::extractInteger(src, &macina_1o2))
	{
		log->log("syntax error, expecting integer parameter [macina_1o2]\n");
		return;
	}


	i32 value = 0;
	rhea::string::parser::toNextValidChar(src);
	if (!rhea::string::parser::extractInteger(src, &value))
	{
		log->log("syntax error, expecting integer parameter [target]\n");
		return;
	}

	if (macina_1o2 >= 1 && macina_1o2 <= 2)
	{
		if (value >= 0 && value <= 360)
		{
			rhea::app::SetAperturaVGrind::ask(ch, proto, (u8)macina_1o2, (u16)value);
			log->log("request sent\n");
		}
		else
			log->log("syntax error, invalid [target].\n");
	}
	else
		log->log("syntax error, invalid [macina_1o2].\n");
}