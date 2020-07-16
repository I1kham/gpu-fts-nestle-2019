#ifndef _ESAPIModuleRaw_h_
#define _ESAPIModuleRaw_h_
#include "ESAPIShared.h"

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

	protected:
		void			virt_handleMsgFromServiceQ	(sShared *shared, const rhea::thread::sMsg &msg);
		void			virt_handleMsgFromSubscriber(sShared *shared, sSubscription &sub, const rhea::thread::sMsg &msg, u16 handlerID);
		void			virt_handleMsgFromCPUBridge	(sShared *shared, cpubridge::sSubscriber &sub, const rhea::thread::sMsg &msg, u16 handlerID);
		void			virt_handleMsg_R_fromRs232	(sShared *shared, sBuffer *b);
		void			virt_handleMsgFromSocket (sShared *shared, OSSocket &sok, u32 userParam);
	};


} // namespace esapi

#endif // _ESAPIModuleRaw_h_
