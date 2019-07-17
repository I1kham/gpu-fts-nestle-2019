#include "CmdHandler_ajaxReqSelAvailability.h"
#include "GUIBridge.h"


using namespace guibridge;

//***********************************************************
void CmdHandler_ajaxReqSelAvailability::handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const char *params UNUSED_PARAM)
{
    rhea::thread::pushMsg (hQMessageToWebserver, GUIBRIDGE_REQ_SELAVAILABILITY, (u32)getHandlerID());
}

//***********************************************************
void CmdHandler_ajaxReqSelAvailability::handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU)
{
    //dataFromGPU contiene le info sullo stato attuale della disp delle selezioni
    //1 byte per indicare il num di selezioni
    //1 bit per ogni selezione
    u8 numSel = dataFromGPU[0];
    const u8 *selAvailiabilityBitArray = &dataFromGPU[1];



    char avail[256];
    u8  byte = 0;
    u8  bit = 0x01;
    for (u8 i=0; i<numSel; i++)
    {
        if ( (selAvailiabilityBitArray[byte] & bit) != 0)
            avail[i] = '1';
        else
            avail[i] = '0';

        if (bit == 0x80)
        {
            byte++;
            bit = 0x01;
        }
        else
            bit <<= 1;
    }
    avail[numSel] = 0x00;


    char resp[256];
    sprintf (resp, "{\"n\":%d,\"s\":\"%s\"}", numSel, avail);
    guibridge::sendAjaxAnwer (server, hClient, ajaxRequestID, resp, strlen(resp));
}
