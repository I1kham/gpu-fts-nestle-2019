#ifndef _CmdHandler_ajaxReqGetTime_h_
#define _CmdHandler_ajaxReqGetTime_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetTime
     *
     * la GUI richiede il valore di tutti i decounterProdotto + i 3 contatori water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     *
        Input:
            command: getTime
            params:  none

        Output:
			json
			{
				h: intero 
				m: intero
				s: intero
			}
     */


    class CmdHandler_ajaxReqGetTime : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqGetTime(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getTime"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetTime_h_
