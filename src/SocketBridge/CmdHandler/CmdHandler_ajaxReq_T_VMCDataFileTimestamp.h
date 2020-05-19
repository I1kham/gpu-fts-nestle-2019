#ifndef _CmdHandler_ajaxReq_T_VMCDataFileTimestamp_h_
#define _CmdHandler_ajaxReq_T_VMCDataFileTimestamp_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_T_VMCDataFileTimestamp
     *
     * Il client ha mandato una richiesta AJAX per conosce il timestamp del da3
     *
        Input:
            command: da3ts
            params:  none

        Output
			stringa
     */


    class CmdHandler_ajaxReq_T_VMCDataFileTimestamp : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_T_VMCDataFileTimestamp(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "da3ts"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_T_VMCDataFileTimestamp_h_
