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

		bool					sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger);
								/*
									in ingresso, [in_out_sizeOfAnswer] contiene la dimensione di out_answer
									in uscita, contiene il num di bytes inseriti in out_answer (ovvero la risposta della CPU)
								*/

		bool					isOpen() const																						{ return true; }

	private:
		struct sRunningSel
		{
			u8	selNum;
			u64	timeStartedMSec;
		};

	private:
		void					priv_buildAnswerTo_checkStatus_B(u8 *out_answer, u16 *in_out_sizeOfAnswer) const;

	private:
		bool						bShowDialogStopSelezione;
		eStatoPreparazioneBevanda	statoPreparazioneBevanda;
		eVMCState					VMCState;
		sRunningSel					runningSel;
    };

} // namespace cpubridge

#endif // _CPUChannelFakeCPU_h_
