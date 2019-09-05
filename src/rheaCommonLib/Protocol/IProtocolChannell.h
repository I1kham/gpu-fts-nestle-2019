#ifndef _rheaIProtocolChannel_h_
#define _rheaIProtocolChannel_h_
#include "Protocol.h"
#include "../rheaLinearBuffer.h"

namespace rhea
{
	/****************************************************************
	*  IProtocolChannell
	*
	*	Astrazione di un canale di comunicazione (es: socket, seriale...)
	*	Espone dei metodi generici (read, write, close) che le classi derivate devono implementare
	*	in modo da astrarre la "fisicità" del dispositivo di comunicazione dal suo funzionamento logico
	*/
	class IProtocolChannell
	{
	public:
							IProtocolChannell (Allocator *allocatorIN, u32 startingSizeOfReadBuffer);	
		virtual				~IProtocolChannell();

		bool				isOpen() const														{ return virt_isOpen(); }
							/* true se il canale è fisicamente aperto ed è quindi legale usare read/wwrite*/

		void				close()																{ virt_close(); }
							/*	chiude fisicamente il canale di comunicazione */

		u16					read (u32 timeoutMSec);
							/*	legge dal canale di comunicazione (non bloccante se timeoutMSec==0) ed eventualmente filla un buffer interno con i bytes ricevuti.
								Ritorna il numero di bytes attualmente presenti nel buffer di lettura.

								In caso di errore, il valore ritornato è >= protocol::RES_ERROR
								Ad esempio, se durante la lettura per qualunque motivo il canale dovesse chiudersi, la fn ritorna protocol::RES_CHANNEL_CLOSED
							*/

		u16					write (const u8 *bufferToWrite, u16 nBytesToWrite, u32 timeoutMSec);
							/*	Prova a scrivere [nBytesToWrite], e ci prova per un tempo massimo di [timeoutMSec].
								Ritorna il numero di bytes scritti sul canale di comunicazione.
								In caso di errore, il valore ritornato è >= protocol::RES_ERROR
								Ad esempio, se durante la scrittra per qualunque motivo il canale dovesse chiudersi, la fn ritorna protocol::RES_CHANNEL_CLOSED
							*/


		const u8*			getReadBuffer() const												{ return rBuffer; }
		u16					getNumBytesInReadBuffer() const										{ return nBytesInReadBuffer; }
		void				consumeReadBuffer(u16 nBytes);
							/*	Shifta il read buffer eliminando i primi nBytes. 
								Aggiorna ovviamente [nBytesInReadBuffer]
							*/

	protected:
		virtual bool		virt_isOpen() const = 0;
		virtual void		virt_close() = 0;
		
		virtual u16			virt_read (u8 *buffer, u16 nMaxBytesToRead, u32 timeoutMSec) = 0;
							/* Legge al massimo [nMaxBytesToRead] e li mette in [buffer].								
							riguardo al valore ritornato, deve seguire la stessa policy di read()
							*/

		virtual u16			virt_write (const u8 *bufferToWrite, u16 nBytesToWrite) = 0;
							/* riguardo al valore ritornato, deve seguire la stessa policy di write() */

	private:
		void				priv_growReadBuffer();

	private:
		Allocator			*allocator;
		u8                  *rBuffer;
		u16                 RBUFFER_SIZE;
		u16                 nBytesInReadBuffer;
	};
} // namespace rhea

#endif // _rheaIProtocolChannel_h_