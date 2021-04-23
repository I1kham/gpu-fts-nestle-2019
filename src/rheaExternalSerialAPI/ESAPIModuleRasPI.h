#ifndef _ESAPIModuleRasPI_h_
#define _ESAPIModuleRasPI_h_
#include "ESAPIShared.h"

namespace esapi
{	
	class ModuleRasPI : public Module
{
	public:
								ModuleRasPI();
								~ModuleRasPI()													{ }

        bool					setup (sShared *shared);
		eExternalModuleType		run();

	protected:
        void                    virt_handleMsgFromServiceQ	(sShared *shared, const rhea::thread::sMsg &msg);
        void                    virt_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID);
        void                    virt_handleMsgFromCPUBridge	(sShared *shared, cpubridge::sSubscriber &sub, const rhea::thread::sMsg &msg, u16 handlerID);
        void                    virt_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b);
        void                    virt_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam);

	private:
        static const u32		SIZE_OF_RS232BUFFEROUT = 2048;
		static const u32		SIZE_OF_SOKBUFFER = 1024;

	private:
        enum class eStato : u8
		{
            boot = 0,
            running = 1
		};

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
		void					priv_unsetup(sShared *shared);

		sConnectedSocket*		priv_2280_findConnectedSocketByUID (u32 uid);
		void					priv_2280_onClientDisconnected (sShared *shared, OSSocket &sok, u32 uid);

		void					boot_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID);
		void					boot_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b);
		bool					priv_boot_waitAnswer (sShared *shared, u8 command, u8 code, u8 fixedMsgLen, u8 whichByteContainsAdditionMsgLen, u8 *answerBuffer, u32 timeoutMSec);
		void					priv_boot_handleFileUpload(sShared *shared, sSubscription *sub);

		void					running_handleMsgFromSubscriber	(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID);
		void					running_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b);
		void					running_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam);

	private:
		eStato					stato;
		u8						*rs232BufferOUT;
		u8						*sokBuffer;
		rhea::FastArray<sConnectedSocket>	sockettList;
		sFileUpload				fileUpload;
	};



} // namespace esapi

#endif // _ESAPIModuleRasPI_h_
