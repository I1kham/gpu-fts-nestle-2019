#ifndef _ESAPIModuleRasPI_h_
#define _ESAPIModuleRasPI_h_
#include "ESAPIModule.h"

namespace esapi
{	
	class ModuleRasPI : public Module
{
	public:
								ModuleRasPI();
								~ModuleRasPI()													{ }

		bool					setup (sGlob *glob);
		eExternalModuleType		run();
		
	private:
		static const u32		WAITLIST_EVENT_FROM_SERVICE_MSGQ	= 0x00400000;
		static const u32		WAITLIST_EVENT_FROM_A_SUBSCRIBER	= 0x00500000;
		
		static const u32		SIZE_OF_RS232BUFFERIN = 4096;
		static const u32		SIZE_OF_RS232BUFFEROUT = 4096;



	private:
		void					priv_unsetup();

		void					priv_boot_run();
		void					priv_boot_handleMsgFromSubscriber(sSubscription *sub);
		bool					priv_boot_waitAnswer (u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec);

		void					priv_handleMsgFromServiceQ();
		

		void					priv_rs232_handleCommunication (sBuffer &b);
		void					priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend);

	private:
		sGlob					*glob;
		OSWaitableGrp           waitableGrp;
		sBuffer					rs232BufferIN;
		u8						*rs232BufferOUT;
		bool					bQuit;

	};



} // namespace esapi

#endif // _ESAPIModuleRasPI_h_
