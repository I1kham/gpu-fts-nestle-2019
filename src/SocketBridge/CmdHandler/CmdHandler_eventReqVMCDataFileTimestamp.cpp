#include "CmdHandler_eventReqVMCDataFileTimestamp.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqVMCDataFileTimestamp::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqVMCDataFileTimestamp::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::CurrentVMCDataFileTimestamp::decodeAnswer()

	cpubridge::sCPUVMCDataFileTimeStamp ts;
	translateNotify_CPU_VMCDATAFILE_TIMESTAMP(msgFromCPUBridge, &ts);


	u8 buffer[32];
	ts.writeToBuffer(buffer);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT,&buffer, ts.getLenInBytes());
}
