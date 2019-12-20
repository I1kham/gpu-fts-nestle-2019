#ifndef _rheaProtocolSocketIO_h_
#define _rheaProtocolSocketIO_h_
#include "IProtocolChannell.h"

namespace rhea
{
	//Attiva questa define per dumpare su file tutto il traffico lungo in canale (viene creato un file diverso per ogni istanza di canale)
	#undef DUMP_CProtocolChSocketTCP_TO_FILE

	//se non siamo in DEBUG, disabilito il DUMP d'ufficio
#ifndef _DEBUG
	#ifdef DUMP_CProtocolChSocketTCP_TO_FILE
		#undef DUMP_CProtocolChSocketTCP_TO_FILE
	#endif
#endif

	/************************************************
	 * ProtocolSocketTCP
	 *	Implementa un canale di comunicazione basato su socket TCP
	 *
	 *	Per i dettagli, vedi IProtocolChannel.h
	 */
	class ProtocolChSocketTCP : public IProtocolChannell
	{
	public:
							ProtocolChSocketTCP(Allocator *allocatorIN, u16 startingSizeOfReadBufferInBytes, u16 maxSizeOfReadBufferInBytes);
		virtual				~ProtocolChSocketTCP();

		void				bindSocket(OSSocket &sokIN)													{ sok = sokIN; }
		OSSocket&			getSocket()																	{ return sok; }

	protected:
		bool				virt_isOpen() const															{ return rhea::socket::isOpen(sok); }
		void				virt_close()																{ rhea::socket::close(sok); }
		u16					virt_read (u8 *buffer, u16 nMaxBytesToRead, u32 timeoutMSec);
		u16					virt_write (const u8 *bufferToWrite, u16 nBytesToWrite);

	private:
		OSSocket			sok;

#ifdef DUMP_CProtocolChSocketTCP_TO_FILE
		static u16			dump_nextDumpFileID;
		FILE				*fDUMP;
#endif

	
	};
} // namespace rhea
#endif // _rheaProtocolSocketIO_h_
