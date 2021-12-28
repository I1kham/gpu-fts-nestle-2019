#ifndef _CmdHandler_ajaxReqLTEModemEnable_h_
#define _CmdHandler_ajaxReqLTEModemEnable_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqLTEModemEnable
     *
     *	Si richiede l'attivazione/disattivazione del modem LTE
     *
        Input:
            command: LTEModemEnable
            params:
				e: 1|0

        Output:
			"OK"
     */
    class CmdHandler_ajaxReqLTEModemEnable : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqLTEModemEnable(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "LTEModemEnable"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqLTEModemEnable_h_
