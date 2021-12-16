#ifndef _CmdHandler_ajaxReq_snack_0x05_exitProg_h_
#define _CmdHandler_ajaxReq_snack_0x05_exitProg_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_snack_0x05_exitProg
     *
     *
        Input:
            command: snackExitProg
            params:  none

		Output
			stringa "OK" | "KO"
     */


    class CmdHandler_ajaxReq_snack_0x05_exitProg : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_snack_0x05_exitProg(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "snackExitProg"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_snack_0x05_exitProg_h_
