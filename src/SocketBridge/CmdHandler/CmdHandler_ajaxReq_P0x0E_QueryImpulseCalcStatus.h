#ifndef _CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus_h_
#define _CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus
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


    class CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "queryImpulseCalcStatus"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus_h_
