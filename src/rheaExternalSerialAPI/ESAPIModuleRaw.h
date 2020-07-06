#ifndef _ESAPIModuleRaw_h_
#define _ESAPIModuleRaw_h_
#include "ESAPIModule.h"

namespace esapi
{
	/**************************************************************************
	 * ModuleRaw
	 *
	 *	Questa classe gestisce la comunicazione seriale nel caso di default, ovvero nel caso in cui sulla seriale non sia collegato alcun modulo
	 *	ESAPI. In questa situazione, la GPU è slave, in attesa di richieste lungo il cavo seriale. Ad ogni richiesta (lecita), il modulo raw risponde
	 *
	 *	Alla eventuale ricezione di un comando #R1, il modulo raw termina e la classe ESAPICore istanzia il modulo appropriato alla gestione del device che
	 *	si è presentato attraverso il comando #R1
	 */
	class ModuleRaw : public Module
	{
	public:
								ModuleRaw();
								~ModuleRaw()													{ }

		bool					setup (sGlob *glob);
		eExternalModuleType		run();
		
	private:
		static const u32		WAITLIST_EVENT_FROM_SERVICE_MSGQ	= 0x00400000;
		static const u32		WAITLIST_EVENT_FROM_A_SUBSCRIBER	= 0x00500000;
		static const u32		WAITLIST_EVENT_FROM_CPUBRIDGE		= 0x00600000;
        static const u32		WAITLIST_RS232                      = 0x00700000;
		static const u32		SIZE_OF_RS232BUFFEROUT = 1024;

		static const u8			API_VERSION_MAJOR = 1;
		static const u8			API_VERSION_MINOR = 0;

	private:
		struct sRunningSel
		{
			cpubridge::eRunningSelStatus	status;
		};

	private:
		void					priv_unsetup();
		bool					priv_subscribeToCPUBridge (const HThreadMsgW &hCPUServiceChannelW);

		void					priv_handleMsgFromServiceQ();
		void					priv_handleMsgFromSubscriber(sSubscription *sub);

		void					priv_handleIncomingMsgFromCPUBridge();
		void					priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg);

		void					priv_rs232_handleCommunication (sBuffer &b);
		void					priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend);
		bool					priv_rs232_handleCommand_A (sBuffer &b);
		bool					priv_rs232_handleCommand_C (sBuffer &b);
		bool					priv_rs232_handleCommand_R (sBuffer &b);
		bool					priv_rs232_handleCommand_S (sBuffer &b);

	private:
		sGlob					*glob;
		cpubridge::sSubscriber	cpuBridgeSubscriber;
		OSWaitableGrp           waitableGrp;
		bool					bIsSubscribedTpCPUBridge;
		sBuffer					rs232BufferIN;
		u8						*rs232BufferOUT;
		sRunningSel				runningSel;
		eExternalModuleType		retCode;
	};


} // namespace esapi

#endif // _ESAPIModuleRaw_h_
