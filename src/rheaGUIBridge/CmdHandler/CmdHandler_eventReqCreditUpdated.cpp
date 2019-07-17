#include "CmdHandler_eventReqCreditUpdated.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqCreditUpdated::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_CREDIT, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCreditUpdated::handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU)
{
    //8 byte "stringa" con il prezzo già formattato con i decimali e la punteggiatura al posto giusto
    u8 buffer[16];
    memcpy (buffer, dataFromGPU, 8);
    buffer[8] = 0;
    guibridge::sendEvent (server, hClient, EVENT_TYPE, buffer, 9);
}

