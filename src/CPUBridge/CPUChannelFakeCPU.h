#ifndef _CPUChannelFakeCPU_h_
#define _CPUChannelFakeCPU_h_
#include "CPUChannel.h"


namespace cpubridge
{
	/*********************************************************
	 * CPUChannelFakeCPU
	 *
	 *	finge di essere una CPU vera e risponde di conseguenza
	 */
	class CPUChannelFakeCPU : public CPUChannel
	{
	public:
								CPUChannelFakeCPU();
								~CPUChannelFakeCPU();

		bool                    open (rhea::ISimpleLogger *logger);
		void                    close (rhea::ISimpleLogger *logger);

		bool					sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec);
								/*
									in ingresso, [in_out_sizeOfAnswer] contiene la dimensione di out_answer
									in uscita, contiene il num di bytes inseriti in out_answer (ovvero la risposta della CPU)
								*/

		bool					isOpen() const																						{ return true; }

        bool					sendOnlyAndDoNotWait(const u8 *bufferToSend UNUSED_PARAM, u16 nBytesToSend UNUSED_PARAM, rhea::ISimpleLogger *logger UNUSED_PARAM)  { return false; }
        bool					waitChar(u64 timeoutMSec UNUSED_PARAM, u8 *out_char UNUSED_PARAM)                                                                   { return false; }
        bool					waitForASpecificChar(u8 expectedChar UNUSED_PARAM, u64 timeoutMSec UNUSED_PARAM)                                                    { return false;  }

	private:
		struct sRunningSel
		{
			u8	selNum;
			u64	timeStartedMSec;
		};

		struct sCleaning
		{
			eCPUProgrammingCommand_cleaningType cleaningType;
			u8									fase;
			u8									btn1;
			u8									btn2;
			eVMCState							prevState;
			u64									timeToEnd;
		};

	private:
		void					priv_buildAnswerTo_checkStatus_B(u8 *out_answer, u16 *in_out_sizeOfAnswer);
		void					priv_updateCPUMessageToBeSent (u64 timeNowMSec);

	private:
		bool						bShowDialogStopSelezione;
		eStatoPreparazioneBevanda	statoPreparazioneBevanda;
		eVMCState					VMCState;
		sRunningSel					runningSel;

		char						cpuMessage1[36];
		char						cpuMessage2[36];
		const char					*curCPUMessage;
		u8							curCPUMessageImportanceLevel;
		u64							timeToSwapCPUMsgMesc;
		sCleaning					cleaning;
    };

} // namespace cpubridge

#endif // _CPUChannelFakeCPU_h_
