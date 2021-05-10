#ifndef _CmdHandler_ajaxReq_P0x06_GetAllDecounterValues_h_
#define _CmdHandler_ajaxReq_P0x06_GetAllDecounterValues_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x06_GetAllDecounterValues
     *
     * la GUI richiede il valore di tutti e 10 i decounterProdotto + i 3 contatori water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     * + il "blocking_counter"
     *
        Input:
            command: getAllDecounters
            params:  none

        Output:
			stringa di 15 interi separati da virgola. I primi 10 sono i decontatori prodotti, poi water_filter_dec, coffee_brewer_dec, coffee_ground_dec,
            blocking_counter, maintenance counter
     */


    class CmdHandler_ajaxReq_P0x06_GetAllDecounterValues : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x06_GetAllDecounterValues(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getAllDecounters"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetProdDecounterValues_h_
