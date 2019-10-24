#ifndef _UserCommand_download_h_
#define _UserCommand_download_h_
#include "UserCommand.h"

/***************************************************
 * UserCommand_download  
 *
 */
class UserCommand_download : public UserCommand
{
public:
					UserCommand_download() : UserCommand ("download") { }

	const char*		getExplain() const;

	void			handle(const char *command, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf) const;

};




#endif // _UserCommand_download_h_
