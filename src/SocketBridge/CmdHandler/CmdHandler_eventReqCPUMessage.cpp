#include "CmdHandler_eventReqCPUMessage.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUMessage::handleRequest (cpubridge::sSubscriber &from, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
	cpubridge::ask_CPU_QUERY_LCD_MESSAGE (from, getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUMessage::handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//rispondo con:
	//1 byte per indicare il livello di importanza del msg
	//2 byte per indicare la lunghezza in byte del messaggio in formato utf8
	//n byte per il messaggio utf8


	cpubridge::sCPULCDMessage cpuMsg;
	cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE(msgFromCPUBridge, &cpuMsg);

	const u16 msgLenInBytes = cpuMsg.ct;

	rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
	u16	bufferSize = 3 + msgLenInBytes;
	u8 *buffer = (u8*)RHEAALLOC(allocator, bufferSize);
	buffer[0] = cpuMsg.importanceLevel;
	buffer[1] = (u8)((msgLenInBytes & 0xFF00) >> 8);
	buffer[2] = (u8)(msgLenInBytes & 0x00FF);
	if (msgLenInBytes > 0)
		memcpy(&buffer[3], cpuMsg.buffer, msgLenInBytes);

	server->sendEvent(hClient, EVENT_TYPE_FROM_SOCKETCLIENT, buffer, bufferSize);

	allocator->dealloc(buffer);
}