#ifndef _rheaProtocol_h_
#define _rheaProtocol_h_
#include "rheaDataTypes.h"
#include "rheaLinearBuffer.h"

namespace rhea
{
    /****************************************************************
     *  IProtocol
     */
    class IProtocol
    {
    public:
                        IProtocol ()                                                              { }
        virtual         ~IProtocol ()                                                             { }

        virtual void    close (OSSocket &sok) = 0;
        virtual u16     read (OSSocket &sok, rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed) = 0;
        virtual i16     writeText (OSSocket &sok, const char *strIN) = 0;
        virtual i16     writeBuffer (OSSocket &sok, const void *bufferIN, u16 nBytesToWrite) = 0;
        virtual void    sendPing(OSSocket &sok) = 0;
        virtual void    sendClose(OSSocket &sok) = 0;
    };

} //namespace rhea

#endif // _rheaProtocol_h_

