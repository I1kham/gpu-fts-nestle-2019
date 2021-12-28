#ifndef _CmdHandler_ajaxReqWiFiSetMode_h_
#define _CmdHandler_ajaxReqWiFiSetMode_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqWiFiSetMode
     *
     *	Si vuole configurare la modalità di funzionamento del wifi
     *
        Input:
            command: wifiSetMode
            params:
				mode: 0|1   => 0=modalità "hotspot", 1=modalità "connectTo"
                SSID:       => se [mode]==connectTo, allora [SSID] indica il nome della rete a cui è connesso
                pwd:        => se [mode]==connectTo, allora [pwd] indica la pwd della connessione

        Output:
			"OK"
     */
    class CmdHandler_ajaxReqWiFiSetMode : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqWiFiSetMode(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "wifiSetMode"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqWiFiSetMode_h_
