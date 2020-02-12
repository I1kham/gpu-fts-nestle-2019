#ifndef _CmdHandler_ajaxReq_P0x1D_ResetEVATotals_
#define _CmdHandler_ajaxReq_P0x1D_ResetEVATotals_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x1D_ResetEVATotals
     *
     * la GUI ha mandato una richiesta AJAX per resettare i parzi<li EVA
     *
        Input:
            command: EVArst

        Output
			OK
			KO
     */
    class CmdHandler_ajaxReq_P0x1D_ResetEVATotals : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_P0x1D_ResetEVATotals(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
								CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
								{ }

		bool				needToPassDownToCPUBridge() const														{ return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "EVArstTotals"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReq_P0x1D_ResetEVATotals_
