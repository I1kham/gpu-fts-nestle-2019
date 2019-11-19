#ifndef _UserCommand_setmotoremacina_h_
#define _UserCommand_setmotoremacina_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_setmotoremacina
 *
 */
class UserCommand_setmotoremacina : public UserCommand
{
public:
					UserCommand_setmotoremacina() : UserCommand ("setmotmacina") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_setmotoremacina_h_
