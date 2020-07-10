#ifndef _CmdHandler_ajaxReq_AliChina_isOnline_h_
#define _CmdHandler_ajaxReq_AliChina_isOnline_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_AliChina_isOnline
     *
     * la GUI ha mandato una richiesta AJAX per sapere se AlipayChina è disponibile oppure no
     *
        Input:
            command: AliChinaIsOnline
            params:
				none

		Output
			OK      -> vuol dire che è online
            oppure
            KO      -> vuol dire che è offline
     */


    class CmdHandler_ajaxReq_AliChina_isOnline : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_AliChina_isOnline(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}


        static const char*  getCommandName()                            { return "AliChinaIsOnline"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_AliChina_isOnline_h_
