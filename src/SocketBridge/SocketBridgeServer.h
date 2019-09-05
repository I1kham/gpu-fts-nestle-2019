#ifndef _SocketBridgeServer_h_
#define _SocketBridgeServer_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "SocketBridgeEnumAndDefine.h"
#include "CommandHandlerList.h"
#include "IdentifiedClientList.h"

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



		void					sendEvent (const HSokServerClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData);
		void					sendAjaxAnwer(const HSokServerClient &h, u8 requestID, const char *ajaxData, u16 lenOfAjaxData);
		
		void					formatSinglePrice (u16 price, char *out, u16 sizeofOut) const;
		void					formatPriceList (const u16 *priceList, u16 nPricesInList, char *out, u16 sizeofOut) const;
									//formatta i primi [] prezzi di [priceList] usando il corretto separatore decimale e numero di cifre dopo il separatore.
									//Il risultato messo in [out] è una stringa coi prezzi separati dal carattere §

    private:
        static const u16        RESERVED_HANDLE_RANGE = 1024;
		

    private:
		bool					priv_decodeMessage(u8 *buffer, u16 nBytesInBuffer, sDecodedMessage *out);

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
        void                    priv_onClientHasDataAvail (u8 iEvent);
        void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
        void                    priv_onConsoleEvent (rhea::thread::sMsg &msg);

	private:
		rhea::ProtocolSocketServer    *server;
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		rhea::NullLogger        nullLogger;
		rhea::LinearBuffer      buffer;
		cpubridge::sSubscriber	subscriber;
		OSEvent					hSubscriberEvent;

		IdentifiedClientList    identifiedClientList;
		CommandHandlerList      cmdHandlerList;
		u64                     nextTimePurgeCmdHandlerList;
		u16                     _nextHandlerID;
		u8						eventSeqNumber;
		bool                    bQuit;
    };

} // namespace socketbridge

#endif // _SocketBridgeServer_h_
