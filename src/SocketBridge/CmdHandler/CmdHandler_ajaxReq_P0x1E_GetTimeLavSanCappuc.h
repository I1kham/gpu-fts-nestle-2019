#ifndef _CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc_h_
#define _CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc
     *
     * la GUI richiede l'orario del prossimo lav sanitario del cappuccinatore
     *
        Input:
            command: getTimeLavSanCapp
            params:  none

        Output:
			json
			{
				h: intero 
				m: intero
			}
     */


    class CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getTimeLavSanCapp"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc_h_
