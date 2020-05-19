#ifndef _CmdHandler_ajaxReq_P0x16_ResetEVA_
#define _CmdHandler_ajaxReq_P0x16_ResetEVA_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x16_ResetEVA
     *
     * la GUI ha mandato una richiesta AJAX per resettare i parziali EVA
     *
        Input:
            command: EVArst

        Output
			OK
			KO
     */
    class CmdHandler_ajaxReq_P0x16_ResetEVA : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_P0x16_ResetEVA(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
								CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
								{ }

		bool				needToPassDownToCPUBridge() const														{ return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "EVArst"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReq_P0x16_ResetEVA_
