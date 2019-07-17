#include "CmdHandler_ajaxReqSelPrices.h"
#include "GUIBridge.h"


using namespace guibridge;

//***********************************************************
void CmdHandler_ajaxReqSelPrices::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const char *params UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_SELPRICES, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqSelPrices::handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU)
{
    //1 byte per indicare il num di selezioni
    //2 byte per la lunghezza della stringa
    //n byte stringa contenenti la lista dei prezzi formattati, separati da §
    u8 numSel = dataFromGPU[0];
    const char *strPriceList = (const char*) &dataFromGPU[3];

    assert (strlen(strPriceList) < 450);

    char resp[512];
    sprintf (resp, "{\"n\":%d,\"s\":\"%s\"}", numSel, strPriceList);
    guibridge::sendAjaxAnwer (server, hClient, ajaxRequestID, resp, strlen(resp));
}
