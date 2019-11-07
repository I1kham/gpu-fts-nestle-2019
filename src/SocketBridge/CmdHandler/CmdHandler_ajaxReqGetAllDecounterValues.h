#ifndef _CmdHandler_ajaxReqGetAllDecounterValues_h_
#define _CmdHandler_ajaxReqGetAllDecounterValues_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetAllDecounterValues
     *
     * la GUI richiede il valore di tutti i decounterProdotto + i 3 contatori water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     *
        Input:
            command: getAllDecounters
            params:  none

        Output:
			stringa di 13 interi separati da virgola. I primi 10 sono i decontatori prodotti, poi water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     */


    class CmdHandler_ajaxReqGetAllDecounterValues : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqGetAllDecounterValues(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getAllDecounters"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetProdDecounterValues_h_
