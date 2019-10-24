#include "CmdHandler_eventReqVMCDataFile.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqVMCDataFile::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_READ_VMCDATAFILE(from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqVMCDataFile::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::ReadVMCDataFile::decodeAnswer()

	cpubridge::eReadDataFileStatus status;
	u16 totKbSoFar;
	u16 fileID;
	cpubridge::translateNotify_READ_VMCDATAFILE_PROGRESS(msgFromCPUBridge, &status, &totKbSoFar, &fileID);


	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(&buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(fileID);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, nbw.length());
}
