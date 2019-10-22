#include "CmdHandler_eventReqCPUbtnProgPressed.h"
#include "../SocketBridge.h"
#include "../../CPUBridge/CPUBridge.h"

using namespace socketbridge;

//***********************************************************
void CmdHandler_eventReqCPUBtnProgPressed::onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge)
{
	//NB: se modifichi questo, modifica anche rhea::app::ButtonProgPressed::decodeAnswer()
	cpubridge::translateNotify_CPU_BTN_PROG_PRESSED(msgFromCPUBridge);
    server->sendEvent (hClient, EVENT_TYPE_FROM_SOCKETCLIENT, NULL, 0);
}
