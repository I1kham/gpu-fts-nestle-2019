#ifndef _CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer_h_
#define _CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer
     *
     * 
     *
        Input:
            command: getCPUStrModelAndVer

        Output
			una stringa in formato UTF8 riportante il nome del master
     */
    class CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getCPUStrModelAndVer"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer_h_
