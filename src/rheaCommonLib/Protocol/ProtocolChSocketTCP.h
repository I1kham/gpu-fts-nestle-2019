#ifndef _rheaProtocolSocketIO_h_
#define _rheaProtocolSocketIO_h_
#include "IProtocolChannell.h"

namespace rhea
{
	/************************************************
	 * ProtocolSocketTCP
	 *	Implementa un canale di comunicazione basato su socket TCP
	 *
	 *	Per i dettagli, vedi IProtocolChannel.h
	 */
	class ProtocolChSocketTCP : public IProtocolChannell
	{
	public:
							ProtocolChSocketTCP (Allocator *allocatorIN, u32 startingSizeOfReadBuffer, OSSocket &sokIN) :
								IProtocolChannell(allocatorIN, startingSizeOfReadBuffer)
							{
								sok = sokIN;
							}

		virtual				~ProtocolChSocketTCP()														{ }

		OSSocket&			getSocket()																	{ return sok; }

	protected:
		bool				virt_isOpen() const															{ return OSSocket_isOpen(sok); }
		void				virt_close()																{ OSSocket_close(sok); }
		u16					virt_read (u8 *buffer, u16 nMaxBytesToRead, u32 timeoutMSec);
		u16					virt_write (const u8 *bufferToWrite, u16 nBytesToWrite);

	private:
		OSSocket			sok;


	
	};
} // namespace rhea
#endif // _rheaProtocolSocketIO_h_
