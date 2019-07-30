#include "CmdHandler_eventReqSelStatus.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqSelStatus::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_SELECTION_REQ_STATUS, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelStatus::handleAnswerToGUI (rhea::ProtocolServer *server, const u8 *dataFromGPU)
{
    //non ci sono dati da inviare, è un semplice evento senza parametri accessori
    u8 status =  dataFromGPU[0];

    guibridge::sendEvent (server, hClient, EVENT_TYPE, &status, 1);
}
