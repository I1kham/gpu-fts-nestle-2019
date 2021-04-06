#ifndef _CmdHandler_ajaxReq_P0x04_SetDecounter_h_
#define _CmdHandler_ajaxReq_P0x04_SetDecounter_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x04_SetDecounter
     *
     * la GUI ha mandato una richiesta AJAX per settare uno dei decounter prodotti
     *
        Input:
            command: setDecounter
            params:
				d: 1..14 vedi enum cpubridge::eCPUProg_decounter
				v: valore a cui resettare (16bit)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReq_P0x04_SetDecounter : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x04_SetDecounter(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "setDecounter"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x04_SetDecounter_h_
