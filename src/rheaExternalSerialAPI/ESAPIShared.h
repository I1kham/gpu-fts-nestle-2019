#ifndef _ESAPIShared_h_
#define _ESAPIShared_h_
#include "ESAPISubscriberList.h"

namespace esapi
{
	class Protocol; //forward decl
	class Module;

	/**************************************************************
	 *
	 */
	class sShared
	{
	public:
		enum eRetCode
		{
			RETCODE_QUIT					= 0x00000001,
			RETCODE_START_MODULE_RASPI		= 0x00000002,
			RETCODE_UNKNOWN					= 0xFFFFFFFF
		};


	public:
							sShared();
							~sShared()						{ }

		eRetCode			run(Module *module);


	public:
		rhea::Allocator		*localAllocator;
		rhea::ISimpleLogger *logger;
		esapi::Protocol		*protocol;

		eRetCode			retCode;

		HThreadMsgR			serviceMsgQR;			//msgQ dalla quale si ricevono msg dagli altri thread
		HThreadMsgW			serviceMsgQW;			//msgQ sulla quale scrivere notifiche che gli altri thread (iscritti) possono leggere
		SubscriberList		subscriberList;			//elenco die thread iscritti a me

		sESAPIModule		moduleInfo;				//informazioni sul modulo ESAPI attualmente connesso alla seriale

		OSWaitableGrp       waitableGrp;

		cpubridge::sSubscriber	cpuBridgeSubscriber;		//da usare per comunicare con CPUBridge

	private:
		void					priv_handleMsgFromServiceQ (Module *module);
		void					priv_handleMsgFromSubscriber(Module *module, sSubscription *sub);
		void					priv_handleRS232 (Module *module);
	};


	/**************************************************************
	 *
	 */
	class Module
	{
	public:
		virtual			~Module()				{ };

		virtual	void	virt_handleMsgFromServiceQ	(sShared *shared, const rhea::thread::sMsg &msg) = 0;
		virtual	void	virt_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID) = 0;
		virtual	void	virt_handleMsgFromCPUBridge	(sShared *shared, cpubridge::sSubscriber &sub, const rhea::thread::sMsg &msg, u16 handlerID) = 0;
		virtual	void	virt_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b) = 0;
		virtual	void	virt_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam) = 0;
	};

} // namespace esapi

#endif // _ESAPIShared_h_
