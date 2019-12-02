#include "CmdHandler_eventReqCPUStatus.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUStatus::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_STATE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUStatus::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::CurrentCPUStatus::decodeAnswer()
	cpubridge::eVMCState vmcState;
	u8 errorCode = 0;
	u8 errorType = 0;
	u16 flag = 0;
	cpubridge::translateNotify_CPU_STATE_CHANGED (msgFromCPUBridge, &vmcState, &errorCode, &errorType, &flag);

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);

	nbw.writeU8((u8)vmcState);
	nbw.writeU8(errorCode);
	nbw.writeU8(errorType);
	nbw.writeU16(flag);

    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
