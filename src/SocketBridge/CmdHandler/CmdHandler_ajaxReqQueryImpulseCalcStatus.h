#ifndef _CmdHandler_ajaxReqQueryImpulseCalcStatus_h_
#define _CmdHandler_ajaxReqQueryImpulseCalcStatus_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqQueryImpulseCalcStatus
     *
     *
        Input:
            command: queryImpulseCalcStatus
            params:  none

        Output
        json
        {
            s: numero intero 8bit che indica lo stato
            v: numero intero 16bit che indica gli impulsi. Sempre == 0 fino a che la CPU sta lavorando. !=0 quando ha finitop
        }
     */


    class CmdHandler_ajaxReqQueryImpulseCalcStatus : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqQueryImpulseCalcStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "queryImpulseCalcStatus"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqQueryImpulseCalcStatus_h_
