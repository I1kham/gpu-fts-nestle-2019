#ifndef _CmdHandler_ajaxReqVMCDataFileTimestamp_h_
#define _CmdHandler_ajaxReqVMCDataFileTimestamp_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqVMCDataFileTimestamp
     *
     * la GUI ha mandato una richiesta AJAX per conosce il timestamp del da3
     *
        Input:
            command: da3ts
            params:  none

        Output
			stringa
     */


    class CmdHandler_ajaxReqVMCDataFileTimestamp : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqVMCDataFileTimestamp(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "da3ts"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqSanWashStatus_h_
