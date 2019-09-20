#ifndef _rheaIProtocol_h_
#define _rheaIProtocol_h_
#include "IProtocolChannell.h"
#include "../SimpleLogger/ISimpleLogger.h"


namespace rhea
{
    /****************************************************************
     *  IProtocol
     *
     *  Interfaccia di base per tutti i protocolli di comunicazione
	 *	Espone dei metodi generici (read, write, close) che le classi derivate devono implementare	
	 *	in modo da astrarre i dettagli implementativi del singolo protocollo da quelle che sono le funzioni di comunicazione
	 *
	 *
     */
    class IProtocol
    {
    public:
						IProtocol (Allocator * allocatorIN, u16 sizeOfWriteBuffer);
		virtual         ~IProtocol();


		virtual bool	handshake_clientSend (IProtocolChannell *ch, rhea::ISimpleLogger *logger = NULL) = 0;
						/* invia l'handshake al server e aspetta la risposta.
						 * Ritorna true se tutto ok, false altrimenti
						 */

		virtual bool	handshake_serverAnswer (IProtocolChannell *ch, rhea::ISimpleLogger *logger = NULL) = 0;
						/* il server si aspetta che il client inizi la connessione con uno specifico handshake.
						 * Se il primi nByte letti dal canale di comunicazione sono un valido handshake, questa fn ritorna il numero
						 * di byte consumati da bufferIN e provvede a rispondere al client, altrimenti ritorna 0
						 */

		void			close (IProtocolChannell *ch)
						{
							/*	Invia eventualmente manda un messaggio di close (se il protocollo lo prevede)*/
							if (ch->isOpen())
								virt_sendCloseMessage(ch);
						}

		u16				read (IProtocolChannell *ch, u32 timeoutMSec, LinearBuffer &out_result);
						/*	legge dal canale di comunicazione (non bloccante se timeoutMSec==0) ed interpreta il buffer di input.
							Se ci sono validi messaggi "utente", ritorna il numero di bytes inseriti in out_result (che corrisponde al primo msg utente estratto dal buffer).
							Eventuali messaggi interni di protocollo (ping, close, keepalive..) vengono gestiti internamente e non
							vengono passati a out_result.

							In caso di errore, il valore ritornato è >= protocol::RES_ERROR;
							Ad esempio, se durante la lettura per qualunque motivo il canale dovesse chiudersi, la fn ritornerebb protocol::RES_CHANNEL_CLOSED.
							Se durante la lettura, riceve un esplicito messaggio di close (di protocollo), ritorna RES_PROTOCOL_CLOSED
							Vedi IProtocolChannell::read per ulteriori dettagli
						*/

		u16				write (IProtocolChannell *ch, const u8 *bufferToWrite, u16 nBytesToWrite, u32 timeoutMSec);
							/*	Prova a scrivere [nBytesToWrite], e ci prova per un tempo massimo di [timeoutMSec].
								Ritorna il numero di bytes scritti sul canale di comunicazione.

								In caso di errore, il valore ritornato è >= protocol::RES_ERROR;
								Ad esempio, se durante la scrittura per qualunque motivo il canale dovesse chiudersi, la fn ritornerebb protocol::RES_CHANNEL_CLOSED.
								Vedi IProtocolChannell::write per ulteriori dettagli
							*/

	protected:
		virtual	void	virt_sendCloseMessage(IProtocolChannell *ch) = 0;

		virtual u16		virt_decodeBuffer (IProtocolChannell *ch, const u8 *bufferIN, u16 nBytesInBufferIN, LinearBuffer &out_result, u16 *out_nBytesInseritiInOutResult) = 0;
						/*	Dato un [bufferIN] che contiene almeno [nBytesInBufferIN] letti da qualche device, questa fn prova ad interpretarne
							il significato secondo il protocollo che implementa.
							Se il buffer contiene almeno un valido pacchetto dati:
								ritorna il numero di bytes di bufferIN "consumati"
								mette in [out_result] il payload del msg
								mette in [out_nBytesInseritiInOutResult] il num di bytes inseriti in out_result (ie: il payloadLen).

							In caso di errore, ritorna >= protocol::RES_ERROR (vedi read)

							Se ritorna 0, vuol dire che in bufferIN non c'era un numero di bytes suff a comporre un intero messaggio, oppure che la funzione
							non è stata in grado di decodificarne il significato.

							ATTENZIONE: la funzione potrebbe ritornare un numero > 0 e contemporaneamente un [out_nBytesInseritiInOutResult]==0. Questo è il caso
							in cui il protocollo implementa dei messaggi interni di comando (per es: ping, keep alive...) per cui di fatto qualcosa di bufferIN è
							stato consumato dalla funzione, ma niente di utile viene inserito in out_result
						*/

		virtual u16		virt_encodeBuffer (const u8 *bufferToEncode, u16 nBytesToEncode, u8 *out_buffer, u16 sizeOfOutBuffer) = 0;
						/*	prende [bufferToEncode] e lo codifica secondo il protocollo che implementa. IL risultato viene messo
							in out_buffer. Ritorna il numero di bytes inseriti in out_buffer
						*/

	protected:
		u8				*bufferW;
		u16				BUFFERW_SIZE;

	private:
		Allocator		*allocator;
	};

} //namespace rhea

#endif // _rheaIProtocol_h_

