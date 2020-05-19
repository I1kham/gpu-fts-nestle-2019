#include "CmdHandler_ajaxReq_T_VMCDataFileTimestamp.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"


using namespace socketbridge;

//***********************************************************
void CmdHandler_ajaxReq_T_VMCDataFileTimestamp::passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params UNUSED_PARAM)
{
	cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(from, getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReq_T_VMCDataFileTimestamp::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questa, modifica anche rhea::app::CurrentVMCDataFileTimestamp::decodeAnswer()
	cpubridge::sCPUVMCDataFileTimeStamp ts;
	translateNotify_CPU_VMCDATAFILE_TIMESTAMP(msgFromCPUBridge, &ts);


	char text[64];
	memset(text, 0, sizeof(text));

	u8 buffer[16];
	ts.writeToBuffer(buffer);
	for (u8 i = 0; i < ts.getLenInBytes(); i++)
	{
		char s[4];
		rhea::string::format::Hex8(buffer[i], s, sizeof(s));

		char ss[16];
		sprintf_s(ss, sizeof(ss), "[%s]", s);
		strcat_s(text, sizeof(text), ss);
	}

    server->sendAjaxAnwer (hClient, ajaxRequestID, (const u8*)text, (u16)strlen(text));
}
