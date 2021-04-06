#ifndef _rheaProtocolConsole_h_
#define _rheaProtocolConsole_h_
#include "IProtocol.h"
#include "../SimpleLogger/ISimpleLogger.h"

namespace rhea
{
    /*************************************************++
     * ProtocolConsole
     *
     */
    class ProtocolConsole : public IProtocol
    {
    public:
        static bool          server_isAValidHandshake (const void *bufferIN, u32 sizeOfBuffer);
                            /* il server si aspetta che il client inizi la connessione con uno specifico handshake.
                             * Se il primi nByte di bufferIN corrispondono ad un valido handshake iniziato dal client con la handshake_clientSend()
                             * allora questa fn ritorna true
                             */

    public:
                            ProtocolConsole (Allocator *allocatorIN, u16 startingSizeOfWriteBuffer, u16 maxSizeOfWriteBuffer) : IProtocol(allocatorIN, startingSizeOfWriteBuffer, maxSizeOfWriteBuffer)		
							{ }
		
		virtual             ~ProtocolConsole()																										
							{ }


		bool				handshake_clientSend (IProtocolChannell *ch, rhea::ISimpleLogger *logger = NULL);
							/* invia l'handshake al server e aspetta la risposta.
							 * Ritorna true se tutto ok, false altrimenti
							 */

		bool				handshake_serverAnswer(IProtocolChannell *ch, rhea::ISimpleLogger *logger = NULL);

	protected:
		void				virt_sendCloseMessage (IProtocolChannell *ch);
		u16					virt_decodeBuffer (IProtocolChannell *ch, const u8 *bufferIN, u16 nBytesInBufferIN, LinearBuffer &out_result, u32 startOffset, u16 *out_nBytesInseritiInOutResult);
		u16					virt_encodeBuffer (const u8 *bufferToEncode, u16 nBytesToEncode, u8 *out_buffer, u16 sizeOfOutBuffer);


    private:
        static const u8     MAGIC_CODE_1 = 0xc8;
        static const u8     MAGIC_CODE_2 = 0xa6;

    private:
        enum class eOpcode : u8
        {
            msg						= 0x01,
            close					= 0x02,
			internal_simpledMsg		= 0xfd,
			internal_extendedMsg	= 0xfe,
            unknown					= 0xff
        };

        struct sDecodeResult
        {
            eOpcode             what;
            const u8            *payload;
            u16                 payloadLen;
        };

	private:
		u16					priv_decodeOneMessage(const u8 *buffer, u16 nBytesInBuffer, sDecodeResult *out_result) const;
		u16                 priv_encodeAMessage(eOpcode opcode, const void *payloadToSend, u16 payloadLen, u8 *out_buffer, u16 sizeOfOutBuffer);

    };

} //namespace rhea
#endif // _rheaProtocolConsole_h_
