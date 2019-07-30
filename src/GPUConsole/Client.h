#ifndef _Client_h_
#define _Client_h_
#include "../rheaCommonLib/Protocol/ProtocolConsole.h"
/********************************************
 * Client
 *
 */
class Client
{
public:
                Client (rhea::Allocator *allocatorIN, OSSocket &sok);
    virtual     ~Client();

    void        close ()                                                                { cp.close(sok); }
    u16         read (rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed)             { return cp.read(sok, out_buffer, bSocketWasClosed); }
    i16         writeBuffer (const void *bufferIN, u16 nBytesToWrite)                   { return cp.writeBuffer(sok, bufferIN, nBytesToWrite); }
    void        sendPing ()                                                             { cp.sendPing(sok); }
    void        sendClose()                                                             { cp.sendClose(sok); }

private:
    OSSocket                sok;
    rhea::ProtocolConsole   cp;
};

#endif // _Client_h_
