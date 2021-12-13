#ifndef _CPUBridgeServer_h_
#define _CPUBridgeServer_h_
#include "CPUBridgeEnumAndDefine.h"
#include "CPUChannel.h"
#include "lang.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "rhFSProtocol.h"
#include "RSProto.h"
#include "CPUBridgeActionScheduler.h"

namespace cpubridge
{
    /**************************************************************************
     * Server
     *
     */
	class Server
	{
	public:
								Server();
								~Server()													{ close(); }

		void                    useLogger(rhea::ISimpleLogger *loggerIN);

		bool                    start (CPUChannel *chToCPU, const HThreadMsgR hServiceChR_IN);
		void					run ();
		void                    close ();


        void                    scheduleAction_rebootASAP();
        void					scheduleAction_relaxedReboot();
        void					scheduleAction_downloadEVADTSAndAnswerToRSProto();

	private:
		static const u16		CPUFW_BLOCK_SIZE	= 400;
		static const u16		RSPROTO_TCP_PORT	= 2283;

	private:
        struct sStato
        {
        public:
            enum class eStato: u8
            {
                comError = 0,
                normal = 1,
                selection = 2,
                programmazione = 3,
				regolazioneAperturaMacina = 4,
				compatibilityCheck = 5,
				CPUNotSupported = 6,
				DA3_sync = 7,
				telemetry = 8,
				grinderSpeedTest = 9,
				downloadPriceHoldingPriceList = 10,

				quit = 0xff
            };


        public:
                        sStato()                        { set(eStato::comError); }
            void        set (eStato s)					{ stato=s; }
            eStato      get() const                     { return stato; }

        private:
            eStato      stato;
        };

		struct sSubscription
		{
			OSEvent		hEvent;
			sSubscriber	q;
		};

		struct sRegolazioneAperturaMacina
		{
			u8 macina_1to4;
			u16 target;
		};
				
		struct sCalcGrinderSpeed
		{
			u8	motoreID;
			u8	tempoDiMacinataInSec;
			u16 lastCalculatedGrinderSpeed;
		};

		enum eStartSelectionMode
		{
			eStartSelectionMode_default			= 0,
			eStartSelectionMode_alreadyPaid		= 1,
			eStartSelectionMode_CPUSpontaneous	= 2,
			eStartSelectionMode_invalid			= 0xff
		};

		struct sStartSelectionParams1
		{
			u8	selNum;
			bool bForceJUG;
		};

		struct sStartSelectionParams2
		{
			u8				selNum;
			bool			bForceJUG;
			u16				price;
			ePaymentMode	paymentMode;
			eGPUPaymentType paymentType;
		};

		struct sStartSelectionParams
		{
		public:
			eStartSelectionMode	how;
			union
			{
				sStartSelectionParams1	asDefault;
				sStartSelectionParams2	asAlreadyPaid;
				sStartSelectionParams1	asCPUSpontaneous;
			};

		public:
			u8	getSelNum() const
			{
				switch (how)
				{
				case eStartSelectionMode_default:
					return this->asDefault.selNum;
				case eStartSelectionMode_alreadyPaid:
					return this->asAlreadyPaid.selNum;
				case eStartSelectionMode_CPUSpontaneous:
					return this->asCPUSpontaneous.selNum;
				default:
					return 0;
				}			
			}

			bool isForceJUG() const
			{
				switch (how)
				{
				case eStartSelectionMode_default:
					return this->asDefault.bForceJUG;
				case eStartSelectionMode_alreadyPaid:
					return this->asAlreadyPaid.bForceJUG;
				case eStartSelectionMode_CPUSpontaneous:
					return false;
				default:
					return false;
				}
			}
		};

		struct sRunningSelection
		{
		public:
			sStartSelectionParams	params;
			//bool					bForceJug;
			const sSubscription		*sub;
			u8						stopSelectionWasRequested;
			eRunningSelStatus		status;

		public:
			u8						getSelNum() const					{ return params.getSelNum(); }
		};

		struct sPriceHolding
		{
			u8		isPriceHolding;		//1 se nel DA3 è impostato il price  holding
			u8		alreadyAsked;		//1 se ho già chiesto i prezzi alla CPU
		};

		struct sJugRepetitions
		{
			u8		jugRepetitions[48];
		};

		struct sIdentifiedTCPClient
		{
			HSokServerClient			hClient;
			u32							customValueOnIdentify;
			rhFSx::proto::eApplicationType	appType;
			u8							verMajor;
			u8							verMinor;
			u8							verBuild;
		};

		struct sScreenMsgOverride
		{
			u16	utf16Msg[sCPULCDMessage::BUFFER_SIZE_IN_U16];
			u64	timeToStopShowingMSec;
		};

	private:
		void					priv_resetInternalState(cpubridge::eVMCState s);
		bool					priv_handleMsgQueues(u64 timeNowMSec UNUSED_PARAM, u32 timeOutMSec);
		void					priv_handleMsgFromServiceMsgQ();
		void					priv_handleMsgFromSingleSubscriber(sSubscription *sub);
		bool					priv_handleProgrammingMessage(sSubscription *sub, u16 handlerID, const rhea::thread::sMsg &msg);

		void					priv_deleteSubscriber (sSubscription *sub, bool bAlsoRemoveFromSubsriberList);
		
		void					priv_enterState_compatibilityCheck();
		void					priv_handleState_compatibilityCheck();

		void					priv_enterState_CPUNotSupported();
		void					priv_handleState_CPUNotSupported();

		void					priv_enterState_DA3Sync();
		void					priv_handleState_DA3Sync();
		void					priv_handleState_DA3Sync_onFinishedOK();

		void					priv_enterState_comError();
		void					priv_handleState_comError();
		void					priv_parseAnswer_initialParam(const u8 *answer, u16 answerLen);

		void					priv_enterState_downloadPriceHoldingPriceList();
		void					priv_handleState_downloadPriceHoldingPriceList();

		void					priv_enterState_normal();
		void					priv_handleState_normal();
		void					priv_parseAnswer_checkStatus(const u8 *answer, u16 answerLen UNUSED_PARAM);

        void					priv_enterState_programmazione();
        void                    priv_handleState_programmazione();

		void					priv_enterState_telemetry();
		void					priv_handleState_telemetry();

		bool					priv_enterState_regolazioneAperturaMacina (u8 macina_1to4, u16 target);
		void                    priv_handleState_regolazioneAperturaMacina();
		bool					priv_sendAndHandleSetMotoreMacina (u8 macina_1to4, eCPUProg_macinaMove m);
		bool					priv_sendAndHandleGetPosizioneMacina(u8 macina_1to4, u16 *out);

		bool					priv_enterState_grinderSpeedTest_AA (u8 macina_1o2, u8 tempoDiMacinataInSec);
		void                    priv_handleState_grinderSpeedTest();

		bool					priv_enterState_selection (const sStartSelectionParams &params, const sSubscription *sub);
		void					priv_handleState_selection();
		void					priv_onSelezioneTerminataKO();

		bool					priv_askVMCDataFileTimeStampAndWaitAnswer(sCPUVMCDataFileTimeStamp *out, u32 timeoutMSec);
		void					priv_updateLocalDA3(const u8 *blockOf64Bytes, u8 blockNum) const;

        u16                     priv_prepareAndSendMsg_checkStatus_B (u8 btnNumberToSend, bool bForceJug);
        bool                    priv_downloadDataAudit_canStartADownload() const                                                                    { return stato.get() == sStato::eStato::normal; }
        eReadDataFileStatus		priv_downloadDataAudit(cpubridge::sSubscriber *subscriber,u16 handlerID, bool bIncludeBufferDataInNotify, u16 *out_fileID);

		void					priv_downloadDataAudit_onFinishedOK(const u8* const fullFilePathAndName, u32 fileID);
		eReadDataFileStatus		priv_downloadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, u16 *out_fileID = NULL);
		eWriteDataFileStatus	priv_uploadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, const u8* const srcFullFileNameAndPath);
		eWriteCPUFWFileStatus	priv_uploadCPUFW (cpubridge::sSubscriber *subscriber, u16 handlerID, const u8* const srcFullFileNameAndPath);
		bool                    priv_prepareSendMsgAndParseAnswer_getExtendedCOnfgInfo_c(sExtendedCPUInfo *out);
        u16                     priv_prepareAndSendMsg_readVMCDataFileBlock (u16 blockNum);
		void					priv_notify_CPU_RUNNING_SEL_STATUS (const sSubscription *sub, u16 handlerID, eRunningSelStatus s);
		
		u8						priv_2DigitHexToInt(const u8 *buffer, u32 index) const;
		bool					priv_WriteByteMasterNext(u8 dato_8, bool isLastFlag, u8 *out_bufferW, u32 &in_out_bufferCT);
		void					priv_retreiveSomeDataFromLocalDA3();

		bool					priv_sendAndWaitAnswerFromCPU (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, u64 timeoutRCVMsec);
		sSubscription*			priv_newSubscription(u16 subscriberUID);

		void					priv_getLockStatusFilename (u8 *out_filePathAndName, u32 sizeOfOutFilePathAndName) const;
		void					priv_writeLockStatus (eLockStatus s) const;
		eLockStatus				priv_getLockStatus() const;
		void					priv_setLockStatus (eLockStatus lockMode);
		bool					priv_setCPUSelectionParam (u8 selNum1ToN, eSelectionParam whichParam, u16 paramValue);
		bool					priv_getCPUSelectionParam (u8 selNum1ToN, eSelectionParam whichParam, u16 *out_paramValue);

		bool					IsSelectionEnable(u8 selNum_1toN);
		bool					SelectionEnable (u8 selNum_1toN, bool bEnable);
		bool					SelectionsEnableLoad();
		bool					SelectionsEnableSave();
		bool					SelectionsEnableFilename(u8* out_filePathAndName, u32 sizeOfOutFilePathAndName) const;

		void					priv_handleEventFromSocket (HSokServerClient hClient);
		bool					priv_onMessageIdentifyRcv (HSokServerClient hClient, const rhFSx::proto::sDecodedMsg &ask);
		void					priv_onTCPClientDisconnected (HSokServerClient hClient);

        eActionResult           priv_runAction_rebootASAP();
        eActionResult           priv_runAction_downloadEVADTSAndAnswerToRSProto();

	private:
		rhea::Allocator				*localAllocator;
		rhea::ISimpleLogger			*logger;
		CPUChannel					*chToCPU;

		rhea::ProtocolSocketServer    *server;
		rhea::LinearBuffer			bufferTCPRead;
		//OSWaitableGrp				waitList;
		RSProto						*rsProto;
        ActionScheduler				actionScheduler;

		rhea::NullLogger			nullLogger;
		HThreadMsgR					hServiceChR;
        sStato						stato;
		u8							answerBuffer[2048];
		sCPUParamIniziali			cpuParamIniziali;
		sCPUStatus					cpuStatus;
		sRunningSelection			runningSel;
		u8							cpu_numDecimalsForPrices;
		rhea::FastArray<sSubscription*>	subscriberList;
		sLanguage					language;
		u16							utf16_lastCPUMsg[sCPULCDMessage::BUFFER_SIZE_IN_U16];
		u16							lastCPUMsg_len;
		u8							lastBtnProgStatus;
        u8							keepOnSendingThisButtonNum;
		sRegolazioneAperturaMacina	regolazioneAperturaMacina;
		sCalcGrinderSpeed			grinderSpeedTest;
		u16							utf16_CPUMasterVersionString[34];
		u64							showCPUStringModelAndVersionUntil_msec;
		sPriceHolding				priceHolding;
		eCPUMilkerType				milkerType;
		u32							id101;
		u16							quickMenuPinCode;
		sScreenMsgOverride			screenMsgOverride;

		u8							jugRepetitions[NUM_MAX_SELECTIONS];
		bool						selectionEnable[NUM_MAX_SELECTIONS];

		friend class ActionScheduler;
    };

} // namespace cpubridge

#endif // _CPUBridgeServer_h_
