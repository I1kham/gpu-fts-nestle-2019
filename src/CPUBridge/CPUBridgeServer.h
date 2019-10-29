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
                eStato_programmazione = 3
            };

            enum eWhatToDo
            {
                eWhatToDo_nothing = 0,
                eWhatToDo_readDataAudit = 1,
            };

        public:
                        sStato()                        { set(eStato_comError, eWhatToDo_nothing); }
            void        set (eStato s, eWhatToDo w)     { stato=s; what=w; }
            eStato      get() const                     { return stato; }
            eWhatToDo   whatToDo() const                { return what; }

        private:
            eStato      stato;
            eWhatToDo   what;
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

	private:

		void					priv_handleMsgQueues(u64 timeNowMSec, u32 timeOutMSec);
		void					priv_handleMsgFromServiceMsgQ();
		void					priv_handleMsgFromSingleSubscriber(sSubscription *sub);
		void					priv_handleProgrammingMessage(sSubscription *sub, eCPUProgrammingCommand cmd, const u8 *optionalData);

		void					priv_deleteSubscriber (sSubscription *sub, bool bAlsoRemoveFromSubsriberList);
		void					priv_enterState_comError();
		void					priv_handleState_comError();
		void					priv_parseAnswer_initialParam(const u8 *answer, u16 answerLen);

		void					priv_enterState_normal();
		void					priv_handleState_normal();
		void					priv_parseAnswer_checkStatus(const u8 *answer, u16 answerLen);

        void					priv_enterState_programmazione();
        void                    priv_handleState_programmazione();

        bool					priv_enterState_selection (u8 selNumber, const sSubscription *sub);
		void					priv_handleState_selection();
		void					priv_onSelezioneTerminataKO();

		bool					priv_askVMCDataFileTimeStampAndWaitAnswer(sCPUVMCDataFileTimeStamp *out);

        u16                     priv_prepareAndSendMsg_checkStatus_B (u8 btnNumberToSend);
        eReadDataFileStatus		priv_downloadDataAudit(cpubridge::sSubscriber *subscriber,u16 handlerID);
		eReadDataFileStatus		priv_downloadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID);
		eWriteDataFileStatus	priv_uploadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath);
		eWriteCPUFWFileStatus	priv_uploadCPUFW (cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath);
		
		u8						priv_2DigitHexToInt(const u8 *buffer, u32 index) const;
		bool					priv_WriteByteMasterNext(u8 dato_8, bool isLastFlag, u8 *out_bufferW, u32 &in_out_bufferCT) const;

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		CPUChannel				*chToCPU;
		OSWaitableGrp			waitList;
		rhea::NullLogger        nullLogger;
		HThreadMsgR             hServiceChR;
		bool					bQuit;
        sStato					stato;
		u8						answerBuffer[256];
		sCPUParamIniziali		cpuParamIniziali;
		sCPUStatus				cpuStatus;
		sRunningSelection		runningSel;
		rhea::FastArray<sSubscription*>	subscriberList;
		sLanguage				language;
		u16						lastCPUMsg[LCD_BUFFER_SIZE_IN_U16];
		u16						lastCPUMsgLen;
		u8						lastBtnProgStatus;
        u8                      keepOnSendingThisButtonNum;
    };

} // namespace cpubridge

#endif // _CPUBridgeServer_h_
