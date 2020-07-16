#ifndef _ESAPICore_h_
#define _ESAPICore_h_
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../CPUBridge/CPUBridge.h"
#include "ESAPIModuleRaw.h"

namespace esapi
{
	/**************************************************
	 *	Core
	 */
	class Core
	{
	public:
								Core();
								~Core()													{ }

		void					useLogger (rhea::ISimpleLogger *loggerIN);
		bool					open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW);
		void					run();

		HThreadMsgW				getServiceMsgQQ() const														{ return shared.serviceMsgQW; }

	private:
		static const u32		WAITLIST_EVENT_FROM_CPUBRIDGE		= 0x00400000;
		static const u32		WAITLIST_EVENT_FROM_SERVICE_MSGQ	= 0x00500000;
		static const u32		WAITLIST_EVENT_FROM_A_SUBSCRIBER	= 0x00600000;
		static const u8			API_VERSION_MAJOR = 1;
		static const u8			API_VERSION_MINOR = 0;
		static const u32		SIZE_OF_RS232BUFFEROUT = 1024;
		static const u32		SIZE_OF_SOKBUFFER = 2048;

		struct sRunningSel
		{
			cpubridge::eRunningSelStatus	status;
		};

		struct sConnectedSocket
		{
			u32			uid;
			OSSocket	sok;
		};



	private:
		void					priv_close();
		void					priv_handleIncomingMsgFromCPUBridge();
		void					priv_handleMsgFromServiceQ();
		void					priv_handleMsgFromSubscriber(sSubscription *sub);
		void					priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg);
		
		void					priv_rs232_handleCommunication (OSSerialPort &comPort, sBuffer &b);
		void					priv_rs232_sendBuffer (OSSerialPort &comPort, const u8 *buffer, u32 numBytesToSend);
		bool					priv_rs232_handleCommand_A (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_C (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_R (OSSerialPort &comPort, sBuffer &b);
		bool					priv_rs232_handleCommand_S (OSSerialPort &comPort, sBuffer &b);

	private:
		sShared					shared;
		rhea::NullLogger        nullLogger;
		bool					bQuit;
		ModuleRaw				*moduleRaw;
	};


} // namespace esapi

#endif // _ESAPICore_h_
