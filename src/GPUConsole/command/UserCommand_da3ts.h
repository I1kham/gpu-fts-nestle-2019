#ifndef _UserCommand_da3ts_h_
#define _UserCommand_da3ts_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_da3ts  (chiede il timestamp del da3)
 *
 */
class UserCommand_da3ts : public UserCommand
{
public:
				UserCommand_da3ts() : UserCommand ("da3ts") { }

	const char*		getExplain() const { return "da3ts => get the current CPU timestamp for the DA3 file"; }
	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
					{
						rhea::app::CurrentVMCDataFileTimestamp::ask(ch, proto);
					}

	
};




#endif // _UserCommand_da3ts_h_
