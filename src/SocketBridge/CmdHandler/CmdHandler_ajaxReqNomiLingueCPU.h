#ifndef _CmdHandler_ajaxReqNomiLingueCPU_h_
#define _CmdHandler_ajaxReqNomiLingueCPU_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqNomiLingueCPU
     *
     * 
     *
        Input:
            command: getCPULangNames

        Output
     */
    class CmdHandler_ajaxReqNomiLingueCPU : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqNomiLingueCPU(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getCPULangNames"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqNomiLingueCPU_h_
