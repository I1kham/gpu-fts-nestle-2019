#ifndef _CPUBridgeServer_h_
#define _CPUBridgeServer_h_
#include "CPUBridgeEnumAndDefine.h"
#include "CPUChannel.h"
#include "lang.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../rheaCommonLib/rheaFastArray.h"

namespace cpubridge
{
	class Server
	{
	public:
								Server();
								~Server()													{ close(); }

		void                    useLogger(rhea::ISimpleLogger *loggerIN);

		bool                    start (CPUChannel *chToCPU, const HThreadMsgR hServiceChR_IN);
		void					run ();
		void                    close ();

	private:
		static const u16		CPUFW_BLOCK_SIZE = 400;

	private:
        struct sStato
        {
        public:
            enum eStato
            {
                eStato_comError = 0,
                eStato_normal = 1,
                eStato_selection = 2,
                eStato_programmazione = 3,
				eStato_regolazioneAperturaMacina = 4,
				eStato_compatibilityCheck = 5,
				eStato_CPUNotSupported = 6,
				eStato_DA3_sync = 7,

				eStato_quit = 0xff
            };


        public:
                        sStato()                        { set(eStato_comError); }
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

		struct sRunningSelection
		{
			u8						selNum;
			const sSubscription		*sub;
			u8	stopSelectionWasRequested;
			eRunningSelStatus		status;
		};

		struct sRegolazioneAperturaMacina
		{
			u8 macina_1o2;
			u16 target;
		};

	private:
		void					priv_resetInternalState(cpubridge::eVMCState s);
		bool					priv_handleMsgQueues(u64 timeNowMSec UNUSED_PARAM, u32 timeOutMSec);
		void					priv_handleMsgFromServiceMsgQ();
		void					priv_handleMsgFromSingleSubscriber(sSubscription *sub);
		void					priv_handleProgrammingMessage(sSubscription *sub, u16 handlerID, const rhea::thread::sMsg &msg);

		void					priv_deleteSubscriber (sSubscription *sub, bool bAlsoRemoveFromSubsriberList);
		
		void					priv_enterState_compatibilityCheck();
		void					priv_handleState_compatibilityCheck();

		void					priv_enterState_CPUNotSupported();
		void					priv_handleState_CPUNotSupported();

		void					priv_enterState_DA3Sync();
		void					priv_handleState_DA3Sync();

		void					priv_enterState_comError();
		void					priv_handleState_comError();
		void					priv_parseAnswer_initialParam(const u8 *answer, u16 answerLen);

		void					priv_enterState_normal();
		void					priv_handleState_normal();
		void					priv_parseAnswer_checkStatus(const u8 *answer, u16 answerLen UNUSED_PARAM);

        void					priv_enterState_programmazione();
        void                    priv_handleState_programmazione();

		bool					priv_enterState_regolazioneAperturaMacina (u8 macina_1o2, u16 target);
		void                    priv_handleState_regolazioneAperturaMacina();
		bool					priv_sendAndHandleSetMotoreMacina (u8 macina_1o2, eCPUProgrammingCommand_macinaMove m);
		bool					priv_sendAndHandleGetPosizioneMacina(u8 macina_1o2, u16 *out);

		bool					priv_enterState_selection (u8 selNumber, const sSubscription *sub);
		void					priv_handleState_selection();
		void					priv_onSelezioneTerminataKO();

		bool					priv_askVMCDataFileTimeStampAndWaitAnswer(sCPUVMCDataFileTimeStamp *out, u32 timeoutMSec);
		void					priv_updateLocalDA3(const u8 *blockOf64Bytes, u8 blockNum) const;

        u16                     priv_prepareAndSendMsg_checkStatus_B (u8 btnNumberToSend);
        eReadDataFileStatus		priv_downloadDataAudit(cpubridge::sSubscriber *subscriber,u16 handlerID);
		void					priv_downloadDataAudit_onFinishedOK(const char *fullFilePathAndName, u32 fileID);
		eReadDataFileStatus		priv_downloadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, u16 *out_fileID = NULL);
		eWriteDataFileStatus	priv_uploadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath);
		eWriteCPUFWFileStatus	priv_uploadCPUFW (cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath);
		bool                    priv_prepareSendMsgAndParseAnswer_getExtendedCOnfgInfo_c(sExtendedCPUInfo *out);
        u16                     priv_prepareAndSendMsg_readVMCDataFileBlock (u16 blockNum);
		
		u8						priv_2DigitHexToInt(const u8 *buffer, u32 index) const;
		bool					priv_WriteByteMasterNext(u8 dato_8, bool isLastFlag, u8 *out_bufferW, u32 &in_out_bufferCT) const;
		void					priv_retreiveSomeDataFromLocalDA3();

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		CPUChannel				*chToCPU;
		OSWaitableGrp			waitList;
		rhea::NullLogger        nullLogger;
		HThreadMsgR             hServiceChR;
        sStato					stato;
		u8						answerBuffer[256];
		sCPUParamIniziali		cpuParamIniziali;
		sCPUStatus				cpuStatus;
		sRunningSelection		runningSel;
		u8						cpu_numDecimalsForPrices;
		rhea::FastArray<sSubscription*>	subscriberList;
		sLanguage				language;
		u16						lastCPUMsg[LCD_BUFFER_SIZE_IN_U16];
		u16						lastCPUMsgLen;
		u8						lastBtnProgStatus;
        u8                      keepOnSendingThisButtonNum;
		sRegolazioneAperturaMacina regolazioneAperturaMacina;
    };

} // namespace cpubridge

#endif // _CPUBridgeServer_h_
