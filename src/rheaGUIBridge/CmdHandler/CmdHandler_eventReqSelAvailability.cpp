#include "CmdHandler_eventReqSelAvailability.h"
#include "GUIBridge.h"
#include "../rheaCommonLib/rheaBit.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqSelAvailability::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_SELAVAILABILITY, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelAvailability::handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU)
{
    //dataFromGPU contiene le info sullo stato attuale della disp delle selezioni
    //1 byte per indicare il num di selezioni
    //1 bit per ogni selezione
    u8 numSel = dataFromGPU[0];
    const u8 *selAvailiabilityBitArray = &dataFromGPU[1];
    const u8 sizeOfSelAvailArray = 1 + (numSel/8);


    //rispondo con:
    //  1 byte per il num sel
    //  1 byte per ogni sel, dove 0x00 =sel non attiva, 0x01=sel attiva
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    u8 *buffer = (u8*)allocator->alloc (1 + numSel);

    memset (buffer, 0x00, numSel+1);
    buffer[0] = numSel;
    for (u8 i=0; i<numSel; i++)
    {
        if (rhea::bit::isSet(selAvailiabilityBitArray, sizeOfSelAvailArray, i))
            buffer[i+1] = 0x01;
    }

    guibridge::sendEvent (server, hClient, EVENT_TYPE, buffer, numSel+1);

    allocator->dealloc(buffer);
}


