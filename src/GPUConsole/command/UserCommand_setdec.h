#ifndef _UserCommand_setdec_h_
#define _UserCommand_setdec_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_setdec
 *
 */
class UserCommand_setdec : public UserCommand
{
public:
					UserCommand_setdec() : UserCommand ("setdec") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_setdec_h_
