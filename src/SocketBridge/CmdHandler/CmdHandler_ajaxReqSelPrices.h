#ifndef _CmdHandler_ajaxReqSelPrices_h_
#define _CmdHandler_ajaxReqSelPrices_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqSelPrices
     *
     * Il client ha mandato una richiesta AJAX per conoscere i prezzi delle selezioni
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
                            CmdHandler_ajaxReqSelPrices (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "selPrice"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqSelPrices_h_
