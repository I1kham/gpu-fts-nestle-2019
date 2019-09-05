#ifndef _CPUBridgeServer_h_
#define _CPUBridgeServer_h_
#include "CPUBridgeEnumAndDefine.h"
#include "CPUChannel.h"
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
		enum eStato
		{
			eStato_comError = 0,
			eStato_normal = 1,
			eStato_selection = 2
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

		void					priv_deleteSubscriber (sSubscription *sub, bool bAlsoRemoveFromSubsriberList);
		void					priv_enterState_comError();
		void					priv_handleState_comError();
		void					priv_parseAnswer_initialParam(const u8 *answer, u16 answerLen);

		void					priv_enterState_normal();
		void					priv_handleState_normal();
		void					priv_parseAnswer_checkStatus(const u8 *answer, u16 answerLen);

		bool					priv_enterState_selection (u8 selNumber, const sSubscription *sub);
		void					priv_handleState_selection();
		void					priv_onSelezioneTerminataKO();

	private:
		rhea::Allocator         *localAllocator;
		rhea::ISimpleLogger     *logger;
		CPUChannel				*chToCPU;
		OSWaitableGrp			waitList;
		rhea::NullLogger        nullLogger;
		HThreadMsgR             hServiceChR;
		bool					bQuit;
		eStato					stato;
		u8						langErrorCode;
		u8						answerBuffer[256];
		sCPUParamIniziali		cpuParamIniziali;
		sCPUStatus				cpuStatus;
		sRunningSelection		runningSel;
		rhea::FastArray<sSubscription*>	subscriberList;

    };

} // namespace cpubridge

#endif // _CPUBridgeServer_h_
