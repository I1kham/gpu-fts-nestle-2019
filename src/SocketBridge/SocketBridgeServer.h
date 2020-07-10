#ifndef _SocketBridgeServer_h_
#define _SocketBridgeServer_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaAlipayChina/AlipayChina.h"
#include "SocketBridgeEnumAndDefine.h"
#include "CommandHandlerList.h"
#include "IdentifiedClientList.h"
#include "SocketBridgeFileT.h"
#include "DBList.h"
#include "SocketBridgeTaskFactory.h"

namespace socketbridge
{
    class Server
    {
    public:
                                Server();
                                ~Server()               { close(); }

        void                    useLogger (rhea::ISimpleLogger *loggerIN);

        bool                    open (u16 serverPort, const HThreadMsgW &hCPUServiceChannelW, bool bDieWhenNoClientConnected);
        void                    close ();
        void                    run ();


		void					sendTo (const HSokServerClient &h, const u8 *buffer, u32 nBytesToSend);
		void					sendEvent (const HSokServerClient &h, eEventType eventType, const void *optionalData, u16 lenOfOptionalData);
		void					sendAjaxAnwer(const HSokServerClient &h, u8 requestID, const u8 *ajaxData, u16 lenOfAjaxData);
		
		void					formatSinglePrice (u16 price, u8 numDecimals, char *out, u16 sizeofOut) const;
		void					formatPriceList (const u16 *priceList, u16 nPricesInList, u8 numDecimals, char *out, u16 sizeofOut) const;
									//formatta i primi [] prezzi di [priceList] usando il corretto separatore decimale e numero di cifre dopo il separatore.
									//Il risultato messo in [out] è una stringa coi prezzi separati dal carattere §

		const IdentifiedClientList*	getIdentifieidClientList() const							{ return &identifiedClientList; }


								//============================== DB ===============================================
		u16						DB_getOrCreateHandle (const u8* const utf8_fullFilePathAndName);
								//ritorna 0 se non è possibile aprire il DB

		const rhea::SQLRst*		DB_q (u16 dbHandle, const u8* const utf8_sql);
								//NULL in caso di errore

		bool					DB_exec (u16 dbHandle, const u8* const utf8_sql);
		void					DB_closeByPath (const u8* const utf8_fullFilePathAndName);
		void					DB_closeByHandle(u16 dbHandle);


								//============================== TASK ===============================================
								template<class TTask>
		void					taskAdd(const char *taskName) { taskFactory->add<TTask>(taskName);  }
		bool					taskSpawnAndRun (const char *taskName, const u8* const params, u32 *out_taskID);
		bool					taskGetStatusAndMesssage (u32 taskID, TaskStatus::eStatus *out_status, char *out_msg, u32 sizeofmsg);


								//============================== ALIPAY China ===============================================
		bool					priv_module_alipayChina_setup ();
		void					module_alipayChina_activate();
		bool					module_alipayChina_hasBeenActivated() const					{ return moduleAlipayChina.subscribed; }
		bool					module_alipayChina_isOnline() const							{ return (moduleAlipayChina.subscribed && moduleAlipayChina.isOnline); }
		bool					module_alipayChina_askQR (const u8 *selectionName, u8 selectionNum, const char *selectionPrice, u8 *out_urlForQRCode, u32 sizeOfOutURL);
		void					module_alipayChina_abort();
		bool					module_getConnectionDetail (char *out_serverIP, u16 *out_serverPort, char *out_machineID, char *out_criptoKey) const;

    private:
        static const u16        RESERVED_HANDLE_RANGE = 1024;
		static const u32		MSGQ_ALIPAY_CHINA = 0xFFFFFFFE;
		
	private:
		struct sModuleAlipayChina
		{
			bool						subscribed;
			bool						threadStarted;
			bool						isOnline;
			HThreadMsgR					hMsgQR;
			HThreadMsgW					hMsgQW;
			rhea::AlipayChina::Context	ctx;
			u8							curSelRunning;
			u16							curSelPrice;
			char						serverIP[16];
			u16							serverPort;
			char						machineID[16];
			char						criptoKey[64];
		};

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

		bool					priv_encodeMessageOfAjax(u8 requestID, const u8 *ajaxData, u16 lenOfAjaxData, u8 *out_buffer, u16 *in_out_bufferLength);
								/* come sopra ma per i comandi di tipo "eOpcode_ajax_A"
								*/

		bool					priv_subsribeToCPU (const HThreadMsgW &hCPUServiceChannelW);
		u16                     priv_getANewHandlerID ();
        void                    priv_onClientHasDataAvail (u8 iEvent, u64 timeNowMSec);
		void					priv_onClientHasDataAvail2(u64 timeNowMSec, HSokServerClient &h, const sIdentifiedClientInfo	*identifiedClient, socketbridge::sDecodedMessage &decoded);
        void                    priv_onCPUBridgeNotification (rhea::thread::sMsg &msg);
		void                    priv_handleIdentification (const HSokServerClient &h, const sIdentifiedClientInfo *identifiedClient, socketbridge::sDecodedMessage &decoded);
		void					priv_onAlipayChinaNotification (rhea::thread::sMsg &msg);

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
		u8						dieWhenNoClientConnected;
		bool                    bQuit;
		u8						cpuBridgeVersion;
		DBList					dbList;
		rhea::SQLRst			rst;

		TaskFactory						*taskFactory;
		rhea::FastArray<TaskStatus*>	runningTask;

		sModuleAlipayChina		moduleAlipayChina;
    };

} // namespace socketbridge

#endif // _SocketBridgeServer_h_
