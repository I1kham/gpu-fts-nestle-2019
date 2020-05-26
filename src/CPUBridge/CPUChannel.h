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
		virtual void            closeAndReopen() = 0;

		virtual bool			sendAndWaitAnswer (const u8 *bufferToSend, u16 nBytesToSend, u8 *out_answer, u16 *in_out_sizeOfAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec) = 0;
                                /*
                                    in ingresso, [in_out_sizeOfAnswer] contiene la dimensione di out_answer
                                    in uscita, contiene il num di bytes inseriti in out_answer (ovvero la risposta della CPU)

                                    Questa fn si aspetta che in [bufferToSend] ci sia un valido msg formattato nel classico protocollo
                                    CPU-GPU, ovvero # [comando] [len] .. [ck]
                                    La fn rimane in attesa di una risposta da parte di CPU. La risposta "valida" è quella che a sua
                                    volta riporta lo stesso [comando] che io ho inviato.
                                    Eventuali risposte ben formattate ma con comando != da [comando], vengono memorizzate e sono accessibili
                                    all'uscita della funzione usando getNumRisposteScartate() e  getBufferOfRisposteScartate()
                                */

		virtual u8				getNumRisposteScartate() const									{ return 0; }
		virtual const u8*		getBufferOfRisposteScartate() const								{ return NULL; }


		virtual bool			isOpen() const = 0;


		virtual bool			sendOnlyAndDoNotWait(const u8 *bufferToSend, u16 nBytesToSend, rhea::ISimpleLogger *logger) = 0;
									//invia ma non aspetta alcuna risposta

		virtual bool			waitChar (u64 timeoutMSec, u8 *out_char) = 0;
									/* prova a leggere un singolo carattere dal canale.
										Se lo trova, ritorna true e mettere il ch in out_char
										Se scade il timeout, ritorna false
									*/
		
		virtual bool			waitForASpecificChar(u8 expectedChar, u64 timeoutMSec) = 0;
									/* legge un carattere. Se questo è == a [expectedChar], allora termina con true, altrimenti prova a leggere un altro char.
										Se entro il timeout non è arrivato il ch desiderato, termina con false
									*/

        virtual u32             waitForAMessage (u8 *out_answer, u32 sizeOf_outAnswer, rhea::ISimpleLogger *logger, u64 timeoutRCVMsec) = 0;
                                    /*  attende la ricezione di un valido messaggio nel formato CPU-GPU
                                        Ritorna il num di byte letti e infilati in out_buffer
                                    */

    };

} // namespace cpubridge

#endif // _CPUChannel_h_
