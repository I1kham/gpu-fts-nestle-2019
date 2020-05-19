#include "CmdHandler_ajaxReq_P0x13_NomiLingueCPU.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x13_NomiLingueCPU::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_GET_NOMI_LINGE_CPU (from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_P0x13_NomiLingueCPU::onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	u16 utf16_lingua1[33];
	u16 utf16_lingua2[33];
	memset (utf16_lingua1, 0, sizeof(utf16_lingua1));
	memset (utf16_lingua2, 0, sizeof(utf16_lingua2));
	cpubridge::translateNotify_NOMI_LINGE_CPU(msgFromCPUBridge, utf16_lingua1, utf16_lingua2);

	rhea::string::utf16::rtrim(utf16_lingua1);
	rhea::string::utf16::rtrim(utf16_lingua2);

	u8 buffer[256];
	rhea::string::strUTF16toUTF8 (utf16_lingua1, buffer, sizeof(buffer));
	rhea::string::utf8::appendUTF8Char (buffer, sizeof(buffer), rhea::UTF8Char('#'));
	
	u32 ct = rhea::string::utf8::lengthInBytes(buffer);
	rhea::string::strUTF16toUTF8 (utf16_lingua2, &buffer[ct], sizeof(buffer) - ct);

	server->sendAjaxAnwer(hClient, ajaxRequestID, buffer, rhea::string::utf8::lengthInBytes(buffer));
}

