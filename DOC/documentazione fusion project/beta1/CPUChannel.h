#ifndef _CPUChannel_h_
#define _CPUChannel_h_
#include "CPUBridgeEnumAndDefine.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"


namespace cpubridge
{
	/*********************************************************
	 * CPUChannel
	 *
	 *	interfaccia per il canale di comunicazione tra CPUBridge e la CPU fisica
	 */
	class CPUChannel
	{
	public:
								CPUChannel()													{}
		virtual					~CPUChannel()													{}

		//bool                    open (const char *COMPORT, rhea::ISimpleLogger *logger);
		virtual void            close(rhea::ISimpleLogger *logger) = 0;

		virtual bool			sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger) = 0;
								/*
									in ingresso, [in_out_sizeOfAnswer] contiene la dimensione di out_answer
									in uscita, contiene il num di bytes inseriti in out_answer (ovvero la risposta della CPU)
								*/

		virtual bool			isOpen() const = 0;
    };

} // namespace cpubridge

#endif // _CPUChannel_h_
