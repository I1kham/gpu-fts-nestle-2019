#ifdef WIN32
#ifndef _winOSSocket_h_
#define _winOSSocket_h_
#include "winOSInclude.h"

namespace platform
{
	void				socket_init (OSSocket *sok);

	eSocketError        socket_openAsTCP (OSSocket *out_sok, int portNumber);

	void                socket_close (OSSocket &sok);

	inline bool			socket_isOpen(const OSSocket &sok)											{ return (sok.socketID != INVALID_SOCKET); }

	inline bool			socket_compare(const OSSocket &a, const OSSocket &b)						{ return (a.socketID == b.socketID); }

	bool                socket_setReadTimeoutMSec(OSSocket &sok, u32 timeoutMSec);

	bool                socket_setWriteTimeoutMSec(OSSocket &sok, u32 timeoutMSec);


	bool                socket_listen(const OSSocket &sok, u16 maxIncomingConnectionQueueLength = u16MAX);
	bool                socket_accept(const OSSocket &sok, OSSocket *out_clientSocket);

	i32                 socket_read(OSSocket &sok, void *buffer, u16 bufferSizeInBytes, u32 timeoutMSec);
	i32                 socket_write(const OSSocket &sok, const void *buffer, u16 nBytesToSend);
}
#endif // _winOSSocket_h_
#endif // WIN32