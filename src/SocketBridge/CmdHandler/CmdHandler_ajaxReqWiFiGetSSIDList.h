#ifndef _CmdHandler_ajaxReqWiFiGetSSIDList_h_
#define _CmdHandler_ajaxReqWiFiGetSSIDList_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqWiFiGetSSIDList
     *
     * E' richiesto l'elenco delle rete wifi visibili
     *
        Input:
            command: wifiGetSSIDList
            params:  none

        Output
            json
            {
                n: numero di SSID trovati
                list: stringa con elenco di SSID separati da \n
            }
     */


    class CmdHandler_ajaxReqWiFiGetSSIDList : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_ajaxReqWiFiGetSSIDList (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "wifiGetSSIDList"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqWiFiGetSSIDList_h_
