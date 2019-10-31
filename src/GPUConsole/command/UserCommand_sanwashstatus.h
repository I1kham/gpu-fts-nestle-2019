#ifndef _UserCommand_sanwashstatus_h_
#define _UserCommand_sanwashstatus_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_sanwashstatus
 *
 */
class UserCommand_sanwashstatus : public UserCommand
{
public:
					UserCommand_sanwashstatus() : UserCommand ("sws") { }

					const char*		getExplain() const			{ return "ask for sanitary washing status\n"; }

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log UNUSED_PARAM, rhea::app::FileTransfer *ftransf UNUSED_PARAM) const
					{
						rhea::app::SanWashingStatus::ask(ch, proto);
					}
};




#endif // _UserCommand_startcleaning_h_
