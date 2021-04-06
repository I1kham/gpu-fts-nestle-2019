#include "CmdHandler_eventReqWriteLocalVMCDataFile.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqWriteLocalVMCDataFile::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen UNUSED_PARAM)
{
	const u8 *localFileName = &payload[1];
	cpubridge::ask_WRITE_VMCDATAFILE(from, getHandlerID(), localFileName);
}

//***********************************************************
void CmdHandler_eventReqWriteLocalVMCDataFile::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::WriteLocalVMCDataFile::decodeAnswer()

	cpubridge::eWriteDataFileStatus status;
	u16 totKbSoFar;
	cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS(msgFromCPUBridge, &status, &totKbSoFar);



	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(&buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
