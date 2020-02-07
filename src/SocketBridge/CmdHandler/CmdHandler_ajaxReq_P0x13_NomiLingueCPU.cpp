#include "CmdHandler_ajaxReq_P0x13_NomiLingueCPU.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"
#include "../../rheaCommonLib/rheaUTF16.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_P0x13_NomiLingueCPU::passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params UNUSED_PARAM)
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

	rhea::utf16::rtrim(utf16_lingua1);
	rhea::utf16::rtrim(utf16_lingua2);

	u8 buffer[256];
	u32 ct = sizeof(buffer);
	rhea::utf16::toUTF8 (utf16_lingua1, buffer, &ct);
	ct--;
	buffer[ct++] = 0x23; //carattere #

	u32 ct2 = sizeof(buffer) - ct;
	rhea::utf16::toUTF8(utf16_lingua2, &buffer[ct], &ct2);

	server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, ct+ct2);

	/*
	u8 buffer[16+(32+1) * 4];
	u8 n = 0;

	//quest 5 byte indicano che la codifica a seguire e' UTF16 invece di UTf8 che e' il default
	buffer[n++] = 0x01;
	buffer[n++] = 0x02;
	buffer[n++] = 0x03;
	buffer[n++] = 0x04;
	buffer[n++] = 0x05;


	for (u8 i = 0;i < 33; i++)
	{
		buffer[n++] = (u8)(strLingua1UTF16[i] & 0x00FF);
		buffer[n++] = (u8)((strLingua1UTF16[i] & 0xFF00) >> 8);
		if (strLingua1UTF16[i] == 0x0000)
			break;
	}
	buffer[n++] = '#';
	buffer[n++] = 0;

	for (u8 i = 0;i < 33; i++)
	{
		buffer[n++] = (u8)(strLingua2UTF16[i] & 0x00FF);
		buffer[n++] = (u8)((strLingua2UTF16[i] & 0xFF00) >> 8);
		if (strLingua2UTF16[i] == 0x0000)
			break;
	}
	server->sendAjaxAnwer(hClient, ajaxRequestID, (const char*)buffer, n);
	*/
}

