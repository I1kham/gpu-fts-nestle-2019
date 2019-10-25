#ifndef _SocketBridgeServer_h_
#define _SocketBridgeServer_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "SocketBridgeEnumAndDefine.h"
#include "CommandHandlerList.h"
#include "IdentifiedClientList.h"
#include "SocketBridgeFileT.h"
#include "DBList.h"

namespace socketbridge
{
    class Server
    {
    public:
                                Server();
                                ~Server()               { close(); }

        void                    useLogger (rhea::ISimpleLogger *loggerIN);

        bool                    open (u16 serverPort, const HThreadMsgW &hCPUServiceChannelW);
        void                    close ();
        void                    run ();


		void					sendTo (const HSokServerClient &h, const u8 *buffer, u32 nBytesToSend);
		void					sendEvent (const HSokServerClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData);
		void					sendAjaxAnwer(const HSokServerClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData);
		
		void					formatSinglePrice (u16 price, char *out, u16 sizeofOut) const;
		void					formatPriceList (const u16 *priceList, u16 nPricesInList, char *out, u16 sizeofOut) const;
									//formatta i primi [] prezzi di [priceList] usando il corretto separatore decimale e numero di cifre dopo il separatore.
									//Il risultato messo in [out] è una stringa coi prezzi separati dal carattere §

		const IdentifiedClientList*	getIdentifieidClientList() const							{ return &identifiedClientList; }


		u16						DB_getOrCreateHandle (const char *fullFilePathAndName);
								//ritorna 0 se non è possibile aprire il DB

		const rhea::SQLRst*		DB_q (u16 dbHandle, const char *sql);
								//NULL in caso di errore

		bool					DB_exec (u16 dbHandle, const char *sql);


    private:
        static const u16        RESERVED_HANDLE_RANGE = 1024;
		
    private:
		bool					priv_extractOneMessage (u8 *buffer, u16 nBytesInBuffer, sDecodedMessage *out, u16 *out_nBytesConsumed) const;

		bool					priv_encodeMessageOfTypeEvent(eEventType eventType, const void *optionalData, u16 lenOfOptionalData, u8 *out_buffer, u16 *in_out_bufferLength);
								/* filla *out_buffer con i bytes necessari per inviare un comando di tipo "eOpcode_event_E".
								 *
								 *	[in_out_bufferLength] inizialmente deve contenere la dimensione in byte di [out_buffer].
								 *
								 *  Ritorna true se tutto ok e, in questo caso, filla [in_out_bufferLength] con il numero di byte inseriti in [out_buffer]
								 *  Ritorna false se il buffer è troppo piccolo e, in questo caso, filla [in_out_bufferLength] con il numero minimo di byte necessari per [out_buffer]
								 */

		bool					priv_encodeMessageOfAjax(u8 requestID, const char *ajaxData, u16 lenOfAjaxData, u8 *out_buffer, u16 *in_out_bufferLength);
								/* come sopra ma per i comandi di tipo "eOpcode_ajax_A"
								*/

		bool					priv_subsribeToCPU (const HThreadMsgW &hCPUServiceChannelW);
		u16                     priv_getANewHandlerID ();
        void                    priv_onClientHasDataAvail (u8 iEvent, u64 timeNowMSec);
		void					priv_onClientHasDataAvail2(u64 timeNowMSec, HSokServerClient &h, const sIdentifiedClientInfo	*identifiedClient, socketbridge::sDecodedMessage &decoded);
        void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
		void                    priv_handleIdentification (const HSokServerClient &h, const sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded);

	private:
		rhea::ProtocolSocketServer    *server;
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		FileTransfer			fileTransfer;
		rhea::NullLogger        nullLogger;
		rhea::LinearBuffer      buffer;
		cpubridge::sSubscriber	subscriber;
		OSEvent					hSubscriberEvent;

		u16						SEND_BUFFER_SIZE;
		u8						*sendBuffer;

		IdentifiedClientList    identifiedClientList;
		CommandHandlerList      cmdHandlerList;
		u64                     nextTimePurgeCmdHandlerList;
		u64						nextTimePurgeDBList;
		u16                     _nextHandlerID;
		u8						eventSeqNumber;
		bool                    bQuit;
		u8						cpuBridgeVersion;
		DBList					dbList;
		rhea::SQLRst			rst;
    };

} // namespace socketbridge

#endif // _SocketBridgeServer_h_
