#ifndef _UserCommand_list_h_
#define _UserCommand_list_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_list
 *
 */
class UserCommand_list : public UserCommand
{
public:
				UserCommand_list() : UserCommand("list") { }

	const char*	getExplain() const														{ return "list => ask a list of currently connected client\n";  }
	
	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::ClientList::ask(ch, proto);
				}

	
};




#endif // _UserCommand_list_h_
