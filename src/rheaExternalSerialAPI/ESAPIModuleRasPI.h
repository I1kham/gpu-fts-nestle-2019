#ifndef _ESAPIModuleRasPI_h_
#define _ESAPIModuleRasPI_h_
#include "ESAPIShared.h"

namespace esapi
{	
	class ModuleRasPI
{
	public:
								ModuleRasPI();
								~ModuleRasPI()													{ }

		bool					setup (sShared *glob);
		eExternalModuleType		run();
		
	private:
		static const u32		WAITLIST_EVENT_FROM_SERVICE_MSGQ	= 0x00400000;
		static const u32		WAITLIST_EVENT_FROM_A_SUBSCRIBER	= 0x00500000;
        static const u32		WAITLIST_RS232                      = 0x00600000;
		
        static const u32		SIZE_OF_RS232BUFFERIN = 2048;
        static const u32		SIZE_OF_RS232BUFFEROUT = 2048;
        static const u32		SIZE_OF_SOKBUFFER = 1024;

	private:
		struct sConnectedSocket
		{
			u32			uid;
			OSSocket	sok;
		};

		struct sFileUpload
		{
			FILE			*f;
			u32				totalFileSizeBytes;
			u16				packetSizeBytes;
			u32				bytesSentSoFar;
			u64				lastTimeUpdatedMSec;
		};


	private:
		void					priv_unsetup();

		void					priv_handleMsgFromServiceQ();
		void					priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend);
		
		sConnectedSocket*		priv_2280_findConnectedSocketByUID (u32 uid);
		void					priv_2280_onClientDisconnected (OSSocket &sok, u32 uid);
		void					priv_2280_sendDataViaRS232 (OSSocket &sok, u32 uid);

		void					priv_boot_run();
		void					priv_boot_handleMsgFromSubscriber(sSubscription *sub);
		bool					priv_boot_waitAnswer (u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec);
		void					priv_boot_handleFileUpload(sSubscription *sub);

		void					priv_running_run();
		void					priv_running_handleMsgFromSubscriber(sSubscription *sub);
		void					priv_running_handleRS232 (sBuffer &b);
		bool					priv_running_handleCommand_R (sBuffer &b);

	private:
		sShared					*shared;
		OSWaitableGrp           waitableGrp;
		sBuffer					rs232BufferIN;
		u8						*rs232BufferOUT;
		u8						*sokBuffer;
		bool					bQuit;
		rhea::FastArray<sConnectedSocket>	sockettList;
		sFileUpload				fileUpload;
	};



} // namespace esapi

#endif // _ESAPIModuleRasPI_h_
