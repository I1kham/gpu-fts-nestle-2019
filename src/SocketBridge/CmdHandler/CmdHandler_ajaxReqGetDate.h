#ifndef _CmdHandler_ajaxReqGetDate_h_
#define _CmdHandler_ajaxReqGetDate_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetDate
     *
     * la GUI richiede il valore di tutti i decounterProdotto + i 3 contatori water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     *
        Input:
            command: getDate
            params:  none

        Output:
			json
			{
				y: intero 
				m: intero
				d: intero
			}
     */


    class CmdHandler_ajaxReqGetDate : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqGetDate(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getDate"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetDate_h_
