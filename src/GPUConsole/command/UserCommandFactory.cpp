#include "UserCommand.h"
#include "UserCommand_list.h"
#include "UserCommand_list.h"
#include "UserCommand_cpumsg.h"
#include "UserCommand_cpustatus.h"
#include "UserCommand_selavail.h"
#include "UserCommand_readDataAudit.h"
#include "UserCommand_cpuiniparam.h"
#include "UserCommand_readDA3.h"
#include "UserCommand_da3ts.h"
#include "UserCommand_download.h"
#include "UserCommand_upload.h"
#include "UserCommand_startsel.h"
#include "UserCommand_startcleaning.h"
#include "UserCommand_cpuprogcmd.h"
#include "UserCommand_sanwashstatus.h"
#include "UserCommand_btnpress.h"

//**********************************************************
void UserCommandFactory::setup (rhea::Allocator *allocatorIN)
{
	allocator = allocatorIN;
	cmdList.setup(allocator, 32);
}

//**********************************************************
void UserCommandFactory::unsetup()
{
	if (NULL == allocator)
		return;

	u32 n = cmdList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		UserCommand *c = cmdList[i];
		RHEADELETE(allocator, c);
	}
	
	cmdList.unsetup();

	allocator = NULL;
}


//**********************************************************
bool UserCommandFactory::handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
{
	u32 n = cmdList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		const char *s = cmdList(i)->getCommandName();
		if (strncmp(command, s, strlen(s)) == 0)
		{
			log->log("sending [%s]...\n", command);
			cmdList(i)->handle(command, ch, proto, log, ftransf);
			return true;
		}
	}

	return false;
}


//**********************************************************
void UserCommandFactory::help_commandLlist(WinTerminal *logger) const
{
	u32 n = cmdList.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		const char *cmdName = cmdList(i)->getCommandName();
		const char *explain = cmdList(i)->getExplain();
		if (NULL == explain)		
			logger->log("%s\n", cmdName);
		else
		{
			char temp[1024];
			strcpy_s(temp, sizeof(temp), explain);
			
			char *pContext;
			char *s = strtok_s(temp, "\n", &pContext);
			if (NULL == s)
				logger->log("%s\n", cmdName);
			else
			{
				logger->log("%-20s%s\n", cmdName, s);
				while ( (s = strtok_s(NULL, "\n", &pContext)) )
					logger->log("                    %s\n", s);
			}
		}
	}
}

//**********************************************************
void UserCommandFactory::utils_addAllKnownCommands()
{
	this->addCommand < UserCommand_btnpress>();
	this->addCommand < UserCommand_cpumsg>();
	this->addCommand < UserCommand_cpustatus>();
	this->addCommand < UserCommand_cpuiniparam>();
	this->addCommand < UserCommand_cpuprogcmd>();
	this->addCommand < UserCommand_list>();
	this->addCommand < UserCommand_readDataAudit>();
	this->addCommand < UserCommand_readDA3>();

	this->addCommand < UserCommand_da3ts>();
	this->addCommand < UserCommand_download>();
	this->addCommand < UserCommand_upload>();
	this->addCommand < UserCommand_sanwashstatus>();
	this->addCommand < UserCommand_selavail>();
	this->addCommand < UserCommand_startsel>();
	this->addCommand < UserCommand_startcleaning>();
}
