#ifndef _CmdHandler_ajaxReqSelPrices_h_
#define _CmdHandler_ajaxReqSelPrices_h_
#include "CmdHandler_ajaxReq.h"


namespace guibridge
{
    /*********************************************************
     * CmdHandler_ajaxReqSelAvailability
     *
     * la GUI ha mandato una richiesta AJAX per conosce la disponibilità delle selezioni
     *
        Input:
            command: selPrice
            params:  none

        Output
        json
        {
            n: numero selezioni
            s: stringa coi prezzi già formattati correttamente, un prezzo per ogni selezione, separati da §
        }
     */


    class CmdHandler_ajaxReqSelPrices : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_ajaxReqSelPrices (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(h, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

        void                handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const char *params);
        void                handleAnswerToGUI (rhea::ProtocolServer *server, const u8 *dataFromGPU);

        static const char*  getCommandName()                            { return "selPrice"; }
    };
} // namespace guibridge
#endif // _CmdHandler_ajaxReqSelPrices_h_
