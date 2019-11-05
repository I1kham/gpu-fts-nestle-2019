#include "CmdHandler_eventReqPartialVMCDataFile.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqPartialVMCDataFile::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen UNUSED_PARAM)
{
	const u8 *buffer64bytes = &payload[1];
	const u8 uno_di = payload[65];
	const u8 tot_num = payload[66];
	const u8 blockOffset = payload[67];
	cpubridge::ask_WRITE_PARTIAL_VMCDATAFILE (from, getHandlerID(), buffer64bytes, uno_di, tot_num, blockOffset);
}

//***********************************************************
void CmdHandler_eventReqPartialVMCDataFile::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::WritePartialVMCDataFile::decodeAnswer()
	u8 blockWritten = 0;
	cpubridge::translateNotify_WRITE_PARTIAL_VMCDATAFILE(msgFromCPUBridge, &blockWritten);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, &blockWritten, 1);
}
