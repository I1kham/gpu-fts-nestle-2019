#ifndef _UserCommand_h_
#define _UserCommand_h_
#include "../../rheaCommonLib/Protocol/IProtocolChannell.h"
#include "../../rheaCommonLib/Protocol/IProtocol.h"
#include "../../rheaAppLib/rheaApp.h"
#include "../../rheaAppLib/rheaAppFileTransfer.h"
#include "../winTerminal.h"

/***************************************************
 * UserCommand
 *
 */
class UserCommand
{
public:
						UserCommand(const char *commandName);
	virtual				~UserCommand()	{ }


	virtual void		handle (const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const = 0;

	const char*			getCommandName() const					{ return command; }

private:
	char				command[32];
};



/*********************************************************
 * UserCommandFactory
 *
 *
 */
class UserCommandFactory
{
public:
						UserCommandFactory()								{ allocator = NULL; }
						~UserCommandFactory()								{ unsetup(); }

	void				setup (rhea::Allocator *allocator);
	void				unsetup();

	void				utils_addAllKnownCommands();
						//fn di comodo, aggiunge tutti i comandi invece che lasciarlo fare all'utente

						template<class TCommand>
	void				addCommand()
						{
							assert(NULL != allocator);
							TCommand *c = RHEANEW(allocator, TCommand)();
							cmdList.append(c);
						}

	bool				handle (const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

	void				help_commandLlist(WinTerminal *logger) const;

private:
	rhea::Allocator					*allocator;
	rhea::FastArray<UserCommand*>	cmdList;


};



#endif // _UserCommand_h_
