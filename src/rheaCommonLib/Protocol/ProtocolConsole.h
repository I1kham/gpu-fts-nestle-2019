#ifndef _ConsoleProtocol_h_
#define _ConsoleProtocol_h_
#include "rheaIProtocol.h"

namespace rhea
{
    /*************************************************++
     * ConsoleProtocol
     *
     */
    class ConsoleProtocol : public IProtocol
    {
    public:
        static bool         client_handshake (OSSocket &sok);
                            /* invia l'handshake al server e aspetta la risposta.
                             * Ritorna true se tutto ok, false altrimenti
                             */

        static i16          server_isValidaHandshake (const void *bufferIN, u32 sizeOfBuffer, OSSocket &sok);
                            /* il server si aspetta che il client inizi la connessione con uno specifico handshake.
                             * Se il primi nByte letti dalla socket dopo la accept() sono un valido handshake, questa fn ritorna il numero
                             * di byte consumati da bufferIN e provvede a rispondere al client, altrimenti ritorna 0
                             */

    public:
                            ConsoleProtocol (rhea::Allocator *allocatorIN, u32 sizeOfWriteBuffer = 512);
        virtual             ~ConsoleProtocol ();

        void                close (OSSocket &sok);
        u16                 read (OSSocket &sok, rhea::LinearBuffer *out_buffer, u8 *bSocketWasClosed);
        i16                 writeBuffer (OSSocket &sok, const void *bufferIN, u16 nBytesToWrite);
        void                sendPing (OSSocket &sok);
        void                sendClose(OSSocket &sok);

    private:
        static const u8     MAGIC_CODE_1 = 0xc8;
        static const u8     MAGIC_CODE_2 = 0xa6;

    private:
        enum eOpcode
        {
            eOpcode_simpleMsg   = 0x01,
            eOpcode_close       = 0x02,
            eOpcode_ping        = 0x03,
            eOpcode_pong        = 0x04,
            eOpcode_unknown     = 0xff
        };

        struct sDecodeResult
        {
            eOpcode             what;
            const u8            *payload;
            u16                 payloadLenInBytes;
        };


    private:
        u16                 priv_decodeBuffer (u8 *rBuffer, u16 nBytesInBuffer, sDecodeResult *out_result) const;
        u16                 priv_encodeBuffer (eOpcode opcode, const void *payloadToSend, u16 payloadLen);
        i16                 priv_sendWBuffer (OSSocket &sok, u16 nBytesToWrite) const;
        void                priv_growReadBuffer();

    private:
        rhea::Allocator     *allocator;
        u8                  *rBuffer;
        u8                  *wBuffer;
        u16                 readBufferSize;
        u16                 nBytesInReadBuffer;
    };

} //namespace rhea
#endif // _ConsoleProtocol_h_
