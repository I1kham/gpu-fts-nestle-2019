#ifndef _CmdHandler_ajaxReqSelAvailability_h_
#define _CmdHandler_ajaxReqSelAvailability_h_
#include "CmdHandler_ajaxReq.h"


namespace guibridge
{
    /*********************************************************
     * CmdHandler_ajaxReqSelAvailability
     *
     * la GUI ha mandato una richiesta AJAX per conosce la disponibilità delle selezioni
     *
        Input:
            command: selAvail
            params:  none

        Output
        json
        {
            n: numero selezioni
            s: stringa di n char (dove n=numero selezioni):
                il char '0' indica che la sel non è disponibile
                il char '1' indica che è disponibile
        }
     */


    class CmdHandler_ajaxReqSelAvailability : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_ajaxReqSelAvailability (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(h, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

        void                handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const char *params);
        void                handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU);

        static const char*  getCommandName()                            { return "selAvail"; }
    };

} // namespace guibridge

#endif // _CmdHandler_ajaxReqSelAvailability_h_
