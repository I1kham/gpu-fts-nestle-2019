#ifndef _ProtocolGUI_h_
#define _ProtocolGUI_h_
#include "IProtocol.h"

namespace rhea
{
    /*************************************************++
     * ProtocolGUI
     *
     */
    class ProtocolWebsocket : public IProtocol
    {
    public:
        static i16          server_isValidaHandshake (char *bufferIN, u32 sizeOfBuffer, OSSocket &sok);
                            /* il server si aspetta che il client inizi la connessione con uno specifico handshake.
                             * Se il primi nByte letti dalla socket dopo la accept() sono un valido handshake, questa fn ritorna il numero
                             * di byte consumati da bufferIN e provvede a rispondere al client, altrimenti ritorna 0
                             */
    public:
                            ProtocolWebsocket (rhea::Allocator *allocatorIN, u32 sizeOfWriteBuffer = 1024);
        virtual             ~ProtocolWebsocket ();

        void                close (OSSocket &sok);
        u16                 read (OSSocket &sok, rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed);
        i16                 writeBuffer (OSSocket &sok, const void *bufferIN, u16 nBytesToWrite);
        i16                 writeText (OSSocket &sok, const char *strIN);
        void                sendPing (OSSocket &sok);
        void                sendClose(OSSocket &sok);

    private:
        static const u32    WRITEBUFFER_SIZE = 512;

        enum eWebSocketOpcode
        {
            eWebSocketOpcode_CONTINUATION = 0x0,
            eWebSocketOpcode_TEXT = 0x1,
            eWebSocketOpcode_BINARY = 0x2,
            eWebSocketOpcode_CLOSE = 0x8,
            eWebSocketOpcode_PING = 0x9,
            eWebSocketOpcode_PONG = 0x0A,
            eWebSocketOpcode_UNKNOWN = 0xff
        };

        struct sDecodeResult
        {
            const u8            *payload;
            u16                 payloadLenInBytes;
            eWebSocketOpcode    opcode;
            bool                bIsLastFrame;
        };

    private:
        u16                 priv_decodeBuffer (u8 *rBuffer, u16 nBytesInBuffer, sDecodeResult *out_result) const;
        u16                 priv_encodeBuffer (bool bFin, eWebSocketOpcode opcode, const void *payloadToSend, u16 payloadLen);
        i16                 priv_sendWBuffer (OSSocket &sok, u16 nBytesToWrite);
        void                priv_growReadBuffer();

    private:
        rhea::Allocator     *allocator;
        u8                  *rBuffer;
        u8                  *wBuffer;
        u16                 readBufferSize;
        u16                 nBytesInReadBuffer;
    };

} //namespace rhea
#endif // _ProtocolGUI_h_
