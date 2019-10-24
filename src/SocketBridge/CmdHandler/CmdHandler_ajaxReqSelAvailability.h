#ifndef _CmdHandler_ajaxReqSelAvailability_h_
#define _CmdHandler_ajaxReqSelAvailability_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
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
                            CmdHandler_ajaxReqSelAvailability (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "selAvail"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqSelAvailability_h_
