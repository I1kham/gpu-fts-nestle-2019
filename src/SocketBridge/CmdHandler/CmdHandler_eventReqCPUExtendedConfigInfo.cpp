#include "CmdHandler_eventReqCPUExtendedConfigInfo.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUExtendedConfigInfo::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUExtendedConfigInfo::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::ExtendedConfigInfo::decodeAnswer()
	
	cpubridge::sExtendedCPUInfo info;
	cpubridge::translateNotify_EXTENDED_CONFIG_INFO(msgFromCPUBridge, &info);
	

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);

	nbw.writeU8(info.msgVersion);
	nbw.writeU8((u8)info.machineType);
	nbw.writeU8(info.machineModel);
	
	server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
