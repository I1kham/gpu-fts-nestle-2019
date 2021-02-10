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
	private:
		static const u8 CPU_REPORTED_PROTOCOL_VERSION = 7;

	public:
								CPUChannelFakeCPU();
								~CPUChannelFakeCPU();

		bool                    open (rhea::ISimpleLogger *logger);
		void                    close (rhea::ISimpleLogger *logger);
		void					closeAndReopen()										{ }

		bool					sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec);

		bool					isOpen() const																						{ return true; }

        bool					sendOnlyAndDoNotWait(const u8 *bufferToSend UNUSED_PARAM, u16 nBytesToSend UNUSED_PARAM, rhea::ISimpleLogger *logger UNUSED_PARAM)  { return false; }
        bool					waitChar(u64 timeoutMSec UNUSED_PARAM, u8 *out_char UNUSED_PARAM)                                                                   { return false; }
        bool					waitForASpecificChar(u8 expectedChar UNUSED_PARAM, u64 timeoutMSec UNUSED_PARAM)                                                    { return false;  }
        u32                     waitForAMessage (u8 *out_answer, u32 sizeOf_outAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec);

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
			u8									freeBuffer8[8];
		};

		struct sMovimentoMacina
		{
			u16			posizioneMacina;
			u8			tipoMovimentoMacina;
			u64			nextTimeUpdateMSec;

			void		reset() { tipoMovimentoMacina = 0; posizioneMacina = 0; nextTimeUpdateMSec = 0; }
			void		update(u64 timeNowMSec)
			{
				if (tipoMovimentoMacina == 0 || timeNowMSec < nextTimeUpdateMSec)
					return;
				nextTimeUpdateMSec = timeNowMSec + 200;
				if (tipoMovimentoMacina == 1)  posizioneMacina++;
				else if (posizioneMacina>0) posizioneMacina--;
			}
		};

		struct sTestModem
		{
			u64	timeToEndMSec;
		};

		struct sTestAssorbGruppo
		{
			u64	timeToEndMSec;
			u8	fase;
			u8	esito;
			u16	result12[12];
		};

	private:
		void					priv_buildAnswerTo_checkStatus_B(u8 *out_answer, u16 *in_out_sizeOfAnswer);
		void					priv_updateCPUMessageToBeSent (u64 timeNowMSec);
		void					priv_DA3_reload();

	private:
		bool						bShowDialogStopSelezione;
		eStatoPreparazioneBevanda	statoPreparazioneBevanda;
		eVMCState					VMCState;
		sRunningSel					runningSel;

		u16							utf16_cpuMessage1[33];
		u16							utf16_cpuMessage2[33];
		const u16					*utf16_curCPUMessage;
		u8							curCPUMessageImportanceLevel;
		u64							timeToSwapCPUMsgMesc;
		sCleaning					cleaning;
		sMovimentoMacina			macine[2];
		sTestModem					testModem;
		u64							timeToEndTestSelezioneMSec;
		sTestAssorbGruppo			testAssorbGruppo;
		u16							decounterVari[32];
		u8							*da3;
    };

} // namespace cpubridge

#endif // _CPUChannelFakeCPU_h_
