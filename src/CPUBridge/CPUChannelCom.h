#ifndef _CPUChannelCom_h_
#define _CPUChannelCom_h_
#include "CPUChannel.h"


namespace cpubridge
{
	/*********************************************************
	 * CPUChannelCom
	 *
	 *	invia messaggi alla CPU usando la seriale
	 */
	class CPUChannelCom : public CPUChannel
	{
	public:
								CPUChannelCom();
								~CPUChannelCom();

		bool                    open (const char *COMPORT, rhea::ISimpleLogger *logger);
		void                    close (rhea::ISimpleLogger *logger);
		void					closeAndReopen();

		bool					sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec);
								/*
									in ingresso, [in_out_sizeOfAnswer] contiene la dimensione di out_answer
									in uscita, contiene il num di bytes inseriti in out_answer (ovvero la risposta della CPU)
								*/

		bool					isOpen() const																						{ return rhea::rs232::isValid(comPort); }

		bool					sendOnlyAndDoNotWait(const u8 *bufferToSend, u16 nBytesToSend, rhea::ISimpleLogger *logger)			{ return priv_handleMsg_send(bufferToSend, nBytesToSend, logger); }
		bool					waitChar(u64 timeoutMSec, u8 *out_char);
		bool					waitForASpecificChar(u8 expectedChar, u64 timeoutMSec);

	private:
		bool					priv_handleMsg_send(const u8 *buffer, u16 nBytesToSend, rhea::ISimpleLogger *logger);
		bool					priv_handleMsg_rcv(u8 commandChar, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec);

	private:
		OSSerialPort			comPort;
		char					sCOMPORT[32];
    };

} // namespace cpubridge

#endif // _CPUChannelCom_h_
