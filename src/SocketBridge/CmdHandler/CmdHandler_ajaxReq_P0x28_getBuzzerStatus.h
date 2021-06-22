#ifndef _CmdHandler_ajaxReq_P0x28_getBuzzerStatus_h_
#define _CmdHandler_ajaxReq_P0x28_getBuzzerStatus_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x28_getBuzzerStatus
     *
     *	GPU vuole conoscere lo stato dell'attuale "operazione di buzzer" iniziata con il comando P0x27
     *
        Input:
            command: getBuzzerStatus
            params:  none

        Output:
			"RUN"  se il buzzer sta ancora lavorando
            "IDLE" se il buzzer non è impegnato
     */


    class CmdHandler_ajaxReq_P0x28_getBuzzerStatus : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x28_getBuzzerStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getBuzzerStatus"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x28_getBuzzerStatus_h_
