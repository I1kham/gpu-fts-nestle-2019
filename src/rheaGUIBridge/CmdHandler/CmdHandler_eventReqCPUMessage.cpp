#include "CmdHandler_eventReqCPUMessage.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqCPUMessage::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_CPU_MESSAGE, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqCPUMessage::handleAnswerToGUI (rhea::ProtocolServer *server, const u8 *dataFromGPU)
{
    //1 byte per indicare il livello di importanza del msg
    //2 byte per indicare la lunghezza in byte del messaggio in formato utf8
    //n byte per il messaggio utf8

    //u8 importanceLevel = dataFromGPU[0];
    u16 msgLenInBytes = dataFromGPU[1];
    msgLenInBytes<<=8;
    msgLenInBytes |= dataFromGPU[2];

    //giro il messaggio paro paro
    guibridge::sendEvent (server, hClient, EVENT_TYPE, dataFromGPU, 3 + msgLenInBytes);
}
