#include "CmdHandler_eventReqStopSel.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqStopSel::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    u32 param32 = getHandlerID();
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_STOPSELECTION, param32);
}

//***********************************************************
void CmdHandler_eventReqStopSel::handleAnswerToGUI (rhea::ProtocolServer *server UNUSED_PARAM, const u8 *dataFromGPU UNUSED_PARAM)
{
    //quest fn non viene mai chiamata, il server non risponde mai ad una richiesta di questo tipo, si limita a segnalare
    //alla GPU che deve far partire una selezione
}
