#ifndef _ESAPIProtocol_h_
#define _ESAPIProtocol_h_
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "ESAPIEnumAndDefine.h"
#include "ESAPIShared.h"
#include "ESAPIDataUpdate.h"

namespace esapi
{
	/**************************************************
	 *	Protocol
	 *
	 *	Implementa i metodi per la gestione dei messaggi in arrivo sulla rs232
	 *	Implementa il parsing dei comandi ESAPI e la logica necessaria per formulare le risposte alle richieste ricevute via rs232
	 */
	class Protocol
	{
	public:
		static const u8			ESAPI_VERSION_MAJOR = 1;
		static const u8			ESAPI_VERSION_MINOR = 3;//2;

	public:
								Protocol();
								~Protocol()														{ priv_unsetup(); }

		bool					setup (rhea::Allocator *allocator, cpubridge::sSubscriber *cpuBridgeSubscriber, const char *serialPort, OSWaitableGrp *waitableGrp, rhea::ISimpleLogger *logger);

		void					rs232_write (const u8 *buffer, u32 numBytesToSend);

		bool					rs232_read ();
									//da chiamarsi per flushare i dati dalla seriale e gestire i msg
									//ritorna false se il messaggio da processare è di tipo 'R'. Questi messaggi sono specifici per lo specifico
									//modulo collegato alla seriale e quindi vanno gestiti in autonomia dal modulo stesso
									//Utilizzare rsr232_getBufferIN() per ottenre il buffer di dati sul quale lavorare per processare il messaggio R

		void					rs232_esapiSendAnswer (u8 c1, u8 c2, const void* optionalData, u32 numOfBytesInOptionalData);

		void					rs232_esapiSendAnswerWithCrc16(u8 c1, u8 c2, const void* optionalData, u32 numOfBytesInOptionalData);


		bool					onMsgFromCPUBridge(cpubridge::sSubscriber &cpuBridgeSubscriber, const rhea::thread::sMsg &msg, u16 handlerID);
								//da chiamarsi ogni volta che CPUBridge manda una notifica
								//ritorna true se ha processato la notifica, false altrimenti

		sBuffer*				rsr232_getBufferIN()												{ return &rs232BufferIN; }
		bool					rsr232_readSingleByte (u8 *out);

	private:
		static const u32		SIZE_OF_BUFFEROUT = 1024;

	private:
		struct sRunningSel
		{
			cpubridge::eRunningSelStatus	status;
		};

	private:
		void					priv_unsetup();
		u32						priv_rs232_handleCommand_A(const sBuffer &b);
		u32						priv_rs232_handleCommand_C(const sBuffer &b);
		u32						priv_rs232_handleCommand_P(const sBuffer& b);
		u32						priv_rs232_handleCommand_S(const sBuffer& b);
		u32						priv_rs232_handleCommand_X(const sBuffer& b);

		void					priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg);

	private:
		rhea::Allocator			*localAllocator;
		rhea::ISimpleLogger		*logger;
		OSWaitableGrp			*waitableGrp;
		cpubridge::sSubscriber	*cpuBridgeSubscriber;
		OSSerialPort			rs232;
		u8						*bufferOUT;
		sBuffer					rs232BufferIN;
		sRunningSel				runningSel;
		bool					commandC8InProgress;

		bool					commandCpuVerExtended;		// selezione del comando A5 di richesta versione CPU in modalita estesa

		DataUpdate				dataUpdate;
	};

} // namespace esapi

#endif // _ESAPIProtocol_h_
