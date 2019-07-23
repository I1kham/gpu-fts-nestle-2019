#include "CmdHandler_eventReqSelPrices.h"
#include "GUIBridge.h"

using namespace guibridge;

//***********************************************************
void CmdHandler_eventReqSelPrices::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_SELPRICES, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_eventReqSelPrices::handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU)
{
    //1 byte per indicare il num di selezioni
    //2 byte per la lunghezza della stringa
    //n byte stringa contenenti la lista dei prezzi formattati, separati da §

    const char *strPriceList = (const char*) &dataFromGPU[3];
    u16 len = strlen(strPriceList);

    //rispondo con la stringa con tutti i prezzi separati da §
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    u8 *buffer = (u8*)allocator->alloc (len);
    memcpy (buffer, strPriceList, len);

    guibridge::sendEvent (server, hClient, EVENT_TYPE, buffer, len);
}



