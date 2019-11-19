#ifndef _UserCommand_getposmacina_h_
#define _UserCommand_getposmacina_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_getposmacina
 *
 */
class UserCommand_getposmacina : public UserCommand
{
public:
					UserCommand_getposmacina() : UserCommand ("getposmacina") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_getposmacina_h_
