#ifndef _CmdHandler_ajaxReq_AliChina_abort_h_
#define _CmdHandler_ajaxReq_AliChina_abort_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_AliChina_abort
     *
     * la GUI ha mandato una richiesta AJAX per abortire il processo di pagamento in corso
     *
        Input:
            command: AliChinaAbort
            params:
				none

		Output
			OK
     */


    class CmdHandler_ajaxReq_AliChina_abort : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_AliChina_abort(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}


        static const char*  getCommandName()                            { return "AliChinaAbort"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_AliChina_abort_h_
