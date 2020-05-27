#ifndef _CmdHandler_eventReqRasPI_getIP_h_
#define _CmdHandler_eventReqRasPI_getIP_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqRasPI_getIP
     *
     * Il client ha mandato una richiesta AJAX per conosce l'IP del modulo rasPI
     *
        Input:
            command: rasPIgetIP
            params:  none

        Output
            stringa con indirizzo a cui puntare per accedere all'interfaccia via wifi
     */


    class CmdHandler_eventReqRasPI_getIP : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_eventReqRasPI_getIP (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "rasPIgetIP"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_eventReqRasPI_getIP_h_