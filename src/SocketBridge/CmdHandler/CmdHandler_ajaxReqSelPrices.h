#ifndef _CmdHandler_ajaxReqSelPrices_h_
#define _CmdHandler_ajaxReqSelPrices_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
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
                            CmdHandler_ajaxReqSelPrices (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(h, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

        void                handleRequest (cpubridge::sSubscriber &from, const char *params);
        void                handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "selPrice"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqSelPrices_h_
