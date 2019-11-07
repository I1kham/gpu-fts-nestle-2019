#ifndef _UserCommand_getalldec_h_
#define _UserCommand_getalldec_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_getalldec
 *
 */
class UserCommand_getalldec : public UserCommand
{
public:
	UserCommand_getalldec() : UserCommand ("getalldec") { }

	const char*	getExplain() const			{ return "ritorna tutti e 13 i decontatori"; }

	void		handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const
				{
					rhea::app::GetAllDecounters::ask(ch, proto);
				}

	
};




#endif // _UserCommand_getalldec_h_
