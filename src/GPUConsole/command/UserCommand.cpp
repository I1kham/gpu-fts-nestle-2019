#include "UserCommand.h"
#include "UserCommand_list.h"



//**********************************************************
UserCommand::UserCommand (const char *commandName)
{
	u32 n = strlen(commandName);
	assert(n < sizeof(command) - 1);
	strcpy(command, commandName);
}
