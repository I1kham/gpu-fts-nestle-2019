#ifndef _CPUBridgeActionScheduler_h_
#define _CPUBridgeActionScheduler_h_
#include "CPUBridgeEnumAndDefine.h"
#include "../rheaCommonLib/rheaFastArray.h"

namespace cpubridge
{
	class Server; //fwd decl

	/**********************************************************************
	 * Action Scheduler
	 * 
	 */
	class ActionScheduler
	{
	public:
		enum class eAction : u8
		{
			//aggiungere action qui implica dover aggiungere il relativo codice di gestione in this->priv_run
            downloadEVADTSandAnswerToRSProto	= 1,
		};

	public:
						ActionScheduler();
						~ActionScheduler();

		void			scheduleReboot (u32 minTimeWaitMSec, u32 checkEveryMSec);
							//attende almeno [minTimeWaitMSec] prima di iniziare a verificare se ci sono le condizioni per un reboot
							//Una volta passato il tempo [minTimeWaitMSec], controlla se è possibile fare reboot ogni [checkEveryMSec]

		void			scheduleAction  (eAction action, u32 firstScheduleInHowManyMsec, u32 scheduleEveryMSec, u32 optionalParam1=0, u32 optionalParam2=0, const void *optionalParam3=NULL);
							// [firstScheduleInHowManyMsec] l'azione viene schedulata fra [firstScheduleInHowManyMsec] msec
							// In caso di rischedulazione, allora viene schedulata ogni [scheduleEveryMSec] dalla fine del run precedente


        void			run (cpubridge::Server *server);
							//esegue tutte le action in lista

		bool			exists (eAction action) const;
							//true se esiste almeno una [action] in lista
	private:
		struct sElem
		{
			eAction	action;
			u32		periodMSec;
			u64		nextTimeCheckMSec;
			u32		param1;
			u32		param2;
			const void *param3;
		};

	private:
        void			priv_run (cpubridge::Server *server);
						

	private:
		rhea::Allocator			*localAllocator;
		rhea::FastArray<sElem>	list;
		u64						rebootNextTimeCheckMSec;
		u32						rebootCheckEveryMSec;
		bool					bIsInsideRunMethod;
	};
} // namespace smu

#endif //_CPUBridgeActionScheduler_h_
