#include "CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"


//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_ARE_YOU_THERE (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_ARE_YOU_THERE, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_ARE_YOU_THERE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 isMITMThere)
{
	logger->log("notify_CPU_RASPI_MITM_ARE_YOU_THERE\n");

	u8 state[1];
	state[0] = isMITMThere;
	rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_ARE_YOU_THERE, handlerID, state, 1);
}

//***************************************************
void cpubridge::translateNotify_CPU_RASPI_MITM_ARE_YOU_THERE(const rhea::thread::sMsg &msg, u8 *isMITMThere)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_RASPI_MITM_ARE_YOU_THERE);
	const u16 *p = (const u16*)msg.buffer;
	*isMITMThere = p[0];
}

//***************************************************
void cpubridge::ask_CPU_RASPI_MITM_START_SOCKETBRIDGE (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RASPI_MITM_START_SOCKETBRIDGE, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_RASPI_MITM_START_SOCKETBRIDGE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_RASPI_MITM_START_SOCKETBRIDGE\n");
	rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RASPI_START_SOCKETBRIDGE, handlerID, NULL, 0);
}


