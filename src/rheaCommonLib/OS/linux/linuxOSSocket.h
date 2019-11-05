#ifdef LINUX
#ifndef _linuxOSSocket_h_
#define _linuxOSSocket_h_
#include "linuxOSInclude.h"

//forward declaration
struct sOSNetAddr;
typedef sOSNetAddr OSNetAddr;

namespace platform
{
	void				socket_init(OSSocket *sok);

    //=============================== TCP
    eSocketError        socket_openAsTCPServer (OSSocket *out_sok, int portNumber);
    eSocketError        socket_openAsTCPClient (OSSocket *out_sok, const char *connectToIP, u32 portNumber);

    void                socket_close (OSSocket &sok);

    inline bool         socket_isOpen (const OSSocket &sok)                                                   { return (sok.socketID > 0); }

    inline bool         socket_compare (const OSSocket &a, const OSSocket &b)                                 { return (a.socketID == b.socketID); }

    bool                socket_setReadTimeoutMSec  (OSSocket &sok, u32 timeoutMSec);

    bool                socket_setWriteTimeoutMSec (OSSocket &sok, u32 timeoutMSec);

    bool                socket_listen (const OSSocket &sok, u16 maxIncomingConnectionQueueLength = u16MAX);
    bool                socket_accept (const OSSocket &sok, OSSocket *out_clientSocket);

    i32                 socket_read (OSSocket &sok, void *buffer, u16 bufferSizeInBytes, u32 timeoutMSec);

    i32                 socket_write(OSSocket &sok, const void *buffer, u16 nBytesToSend);

    //=============================== UDP
    eSocketError        socket_openAsUDP (OSSocket *out_sok);
    eSocketError        socket_UDPbind (OSSocket &sok, int portNumber);
    void				socket_UDPSendBroadcast (OSSocket &sok, const u8 *buffer, u32 nBytesToSend, int portNumber, const char *subnetMask);
    u32					socket_UDPSendTo (OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const OSNetAddr &addrTo);
    u32					socket_UDPReceiveFrom (OSSocket &sok, u8 *buffer, u32 nMaxBytesToRead, OSNetAddr *out_addrFrom);
}

#endif // _linuxOSSocket_h_

#endif
