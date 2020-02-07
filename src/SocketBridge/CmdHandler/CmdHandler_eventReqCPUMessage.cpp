#include "CmdHandler_eventReqCPUMessage.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaUTF16.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUMessage::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_LCD_MESSAGE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUMessage::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//rispondo con:
	//1 byte per indicare il livello di importanza del msg
	//2 byte per indicare la lunghezza in byte del messaggio in formato utf16
	//n byte per il messaggio utf16

	//NB: se modifichi questo, modifica anche rhea::app::CurrentCPUMessage::decodeAnswer()

	cpubridge::sCPULCDMessage cpuMsg;
	cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE (msgFromCPUBridge, &cpuMsg);

	const u16 msgLenInBytes = 2 * rhea::utf16::length(cpuMsg.utf16LCDString);

	rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
	u16	bufferSize = 3 + msgLenInBytes;
	u8 *buffer = (u8*)RHEAALLOC(allocator, bufferSize);
	buffer[0] = cpuMsg.importanceLevel;
	buffer[1] = (u8)((msgLenInBytes & 0xFF00) >> 8);
	buffer[2] = (u8)(msgLenInBytes & 0x00FF);
	if (msgLenInBytes > 0)
		memcpy(&buffer[3], cpuMsg.utf16LCDString, msgLenInBytes);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, bufferSize);

	allocator->dealloc(buffer);
}