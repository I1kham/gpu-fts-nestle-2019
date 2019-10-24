#ifndef _UserCommand_upnload_h_
#define _UserCommand_upload_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_upload
 *
 */
class UserCommand_upload : public UserCommand
{
public:
					UserCommand_upload() : UserCommand ("upload") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_upload_h_
