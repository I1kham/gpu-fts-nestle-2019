#ifndef _CmdHandler_ajaxReq_P0x13_NomiLingueCPU_h_
#define _CmdHandler_ajaxReq_P0x13_NomiLingueCPU_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x13_NomiLingueCPU
     *
     * 
     *
        Input:
            command: getCPULangNames

        Output
     */
    class CmdHandler_ajaxReq_P0x13_NomiLingueCPU : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_P0x13_NomiLingueCPU(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getCPULangNames"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReq_P0x13_NomiLingueCPU_h_
