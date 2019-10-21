#ifndef _UserCommand_readDataAudit_h_
#define _UserCommand_readDataAudit_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_cls
 *
 */
class UserCommand_readDataAudit : public UserCommand
{
public:
				UserCommand_readDataAudit() : UserCommand ("readDataAudit") { }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::ReadDataAudit::ask(ch, proto);
				}

	
};




#endif // _UserCommand_readDataAudit_h_
