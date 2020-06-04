#ifndef _CmdHandler_eventReqRasPI_getIPandSSID_h_
#define _CmdHandler_eventReqRasPI_getIPandSSID_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqRasPI_getIPandSSID
     *
     * Il client ha mandato una richiesta AJAX per conosce l'IP del modulo rasPI
     *
        Input:
            command: rasPIgetIP
            params:  none

        Output
        json
        {
            ip: xxx.yyy.zz.kkk
            ssid: stringa con ssid
        }
     */


    class CmdHandler_eventReqRasPI_getIPandSSID : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_eventReqRasPI_getIPandSSID (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "rasPIgetIPandSSID"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_eventReqRasPI_getIPandSSID_h_