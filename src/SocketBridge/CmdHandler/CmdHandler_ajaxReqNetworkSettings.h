#ifndef _CmdHandler_ajaxReqNetworkSettings_h_
#define _CmdHandler_ajaxReqNetworkSettings_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqNetworkSettings
     *
     * Il client ha mandato una richiesta AJAX per conosce i vari parametri di rete.
     * Valido solo per macchine con motherboard D23
     *
        Input:
            command: getNetworkSettings
            params:  none

        Output
		    json
		    {
			    lanIP: stringa con IP di LAN
                mac: stringa con mac address
			    modemLTE: 0|1       => 0=non attivo, 1=attivo

                wifiMode: 0|1       => 0=modalità hot spot, 1=modalità connect to
                hotSpotSSID:        => se [wifiMode]==hotspot, allora [hotSpotSSID] = stringa con hot spot SSID creato
			    wifiIP:             => se [wifiMode]==connectTo, allora [wifiIP] = stringa con IP di wi-fi che è stato assegnato a questa macchina
                wifiSSID:           => se [wifiMode]==connectTo, allora [wifiSSID] indica il nome della rete a cui è connesso
                wifiPwd:            => se [wifiMode]==connectTo, allora [wifiPwd] indica la pwd della connessione
                wifiConnected: 0|1  => se [wifiMode]==connectTo, allora indica se la connessione è stabilita o no
		    }
     */


    class CmdHandler_ajaxReqNetworkSettings : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqNetworkSettings(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getNetworkSettings"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqNetworkSettings_h_
