#include "CmdHandler_eventReqStartSel.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqStartSel::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen)
{
    assert (payloadLen > 1);
    u8 selNum = payload[1];

    u32 param32 = selNum;
    param32 <<= 16;
    param32 |= getHandlerID();

    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_STARTSELECTION, param32);
}

//***********************************************************
void CmdHandler_eventReqStartSel::handleAnswerToGUI (rhea::ProtocolServer *server UNUSED_PARAM, const u8 *dataFromGPU UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il server non risponde mai ad una richiesta di questo tipo, si limita a segnalare
    //alla GPU che deve far partire una selezione
}
