#include "CmdHandler_eventReqCPUIniParam.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUIniParam::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_INI_PARAM (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUIniParam::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::CurrentCPUInitParam::decodeAnswer()
	
	cpubridge::sCPUParamIniziali iniParam;
	cpubridge::translateNotify_CPU_INI_PARAM(msgFromCPUBridge, &iniParam);
	

	u8 buffer[sizeof(cpubridge::sCPUParamIniziali) + 32];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);

	nbw.writeBlob(iniParam.CPU_version, sizeof(iniParam.CPU_version));
	nbw.writeU8(iniParam.protocol_version);
	for (u8 i = 0; i < 48; i++)
		nbw.writeU16(iniParam.prices[i]);
	
	server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
