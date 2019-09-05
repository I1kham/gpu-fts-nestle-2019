#ifndef _rheaProtocolWebsocket_h_
#define _rheaProtocolWebsocket_h_
#include "IProtocol.h"

namespace rhea
{
    /*************************************************++
     * ProtocolWebsocket
     *
     */
    class ProtocolWebsocket : public IProtocol
    {
    public:
        static bool          server_isAValidHandshake(const void *bufferIN, u32 sizeOfBuffer);
							/* il server si aspetta che il client inizi la connessione con uno specifico handshake.
							 * Se il primi nByte di bufferIN corrispondono ad un valido handshake iniziato dal client con la handshake_clientSend()
							 * allora questa fn ritorna true
							 */
    public:
							ProtocolWebsocket (rhea::Allocator *allocatorIN, u32 sizeOfWriteBuffer = 1024) : IProtocol(allocatorIN, sizeOfWriteBuffer)		{ }
		virtual             ~ProtocolWebsocket()																											{ }

		bool				handshake_clientSend (IProtocolChannell *ch, rhea::ISimpleLogger *logger);
		bool				handshake_serverAnswer(IProtocolChannell *ch, rhea::ISimpleLogger *logger);

							//specifiche del protocollo websocket, non ereditate da IProtocol
		i16                 writeBuffer (IProtocolChannell *ch, const void *bufferIN, u16 nBytesToWrite);
		i16                 writeText (IProtocolChannell *ch, const char *strIN);
		void                sendPing (IProtocolChannell *ch);
		void				sendClose(IProtocolChannell *ch);


	protected:
		void				virt_sendCloseMessage(IProtocolChannell *ch)																											{ sendClose(ch); }
		u16					virt_decodeBuffer (IProtocolChannell *ch, const u8 *bufferIN, u16 nBytesInBufferIN, LinearBuffer &out_result, u16 *out_nBytesInseritiInOutResult);
		u16					virt_encodeBuffer (const u8 *bufferToEncode, u16 nBytesToEncode, u8 *out_buffer, u16 sizeOfOutBuffer);

    private:
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
            u16                 payloadLen;
            eWebSocketOpcode    opcode;
            u8	                bIsLastFrame;
			u8					isMasked;
			u8					keys[4];
        };

		struct Handshake
		{
			char    resource[32];
			char    host[64];
			char    received_key[256];
			char    extension[256];
			char    protocol[128];
			u8      upgrade;
			u8      connection;
			u32     version;
		};

    private:
		u16					priv_decodeOneMessage (const u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const;
        u16                 priv_encodeAMessage (bool bFin, eWebSocketOpcode opcode, const void *payloadToSend, u16 payloadLen, u8 *out_buffer, u16 sizeOfOutBuffer) const;

		static bool         priv_server_isAValidHandshake(const void *bufferIN, u32 sizeOfBuffer, Handshake *out);
	};

} //namespace rhea
#endif // _rheaProtocolWebsocket_h_
