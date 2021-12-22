#include "CPUBridgeActionScheduler.h"
#include "CPUBridgeServer.h"

using namespace cpubridge;


//******************************************* 
ActionScheduler::ActionScheduler()
{
	localAllocator = rhea::getSysHeapAllocator();
	list.setup (localAllocator, 16);

	bIsInsideRunMethod = false;
	rebootNextTimeCheckMSec = rebootCheckEveryMSec = 0;
}

//******************************************* 
ActionScheduler::~ActionScheduler()
{
	list.unsetup();
}

//******************************************* 
void ActionScheduler::scheduleReboot (u32 minTimeWaitMSec, u32 checkEveryMSec)
{
	rebootNextTimeCheckMSec = rhea::getTimeNowMSec() + minTimeWaitMSec;
	rebootCheckEveryMSec = checkEveryMSec;
}


//******************************************* 
void ActionScheduler::scheduleAction  (eAction action, u32 firstScheduleInHowManyMsec, u32 scheduleEveryMSec, u32 optionalParam1, u32 optionalParam2, const void *optionalParam3)
{
	sElem elem;
	elem.action = action;
	elem.nextTimeCheckMSec = rhea::getTimeNowMSec() + firstScheduleInHowManyMsec;
	elem.periodMSec = scheduleEveryMSec;
	elem.param1 = optionalParam1;
	elem.param2 = optionalParam2;
	elem.param3 = optionalParam3;

	list.append(elem);
}

//******************************************* 
bool ActionScheduler::exists (eAction action) const
{
	const u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (list(i).action == action)
			return true;
	}
	return false;
}

//******************************************* 
void ActionScheduler::run (cpubridge::Server *server)
{
	if (bIsInsideRunMethod)
		return;
	bIsInsideRunMethod = true;
	priv_run(server);
	bIsInsideRunMethod = false;
}

//******************************************* 
void ActionScheduler::priv_run (cpubridge::Server *server)
{
	const u64 timeNowMSec = rhea::getTimeNowMSec();

	//se c'è un reboot in lista, verifico il da farsi
	if (rebootNextTimeCheckMSec != 0)
	{
		if (timeNowMSec >= rebootNextTimeCheckMSec)
		{
			rebootNextTimeCheckMSec = timeNowMSec + rebootCheckEveryMSec;
			if (eActionResult::finished == server->priv_runAction_rebootASAP())
				return;
		}
	}

#define HANDLE_ACTION(fnToCall)\
			if (eActionResult::finished ==  fnToCall )	\
				{\
					list.removeAndSwapWithLast(i);\
					--i;\
				}\
				else\
					list[i].nextTimeCheckMSec = timeNowMSec + list(i).periodMSec;\



	//verifico se ci sono action da eseguire
	for (u32 i = 0; i < list.getNElem(); i++)
	{
		if (timeNowMSec >= list(i).nextTimeCheckMSec)
		{
			switch (list(i).action)
			{
			default:
				DBGBREAK;
				break;

			case eAction::downloadEVADTSandAnswerToRSProto:
				HANDLE_ACTION(server->priv_runAction_downloadEVADTSAndAnswerToRSProto());
				break;
			}
		}
	}

#undef HANDLE_ACTION
}



