#ifndef _CmdHandler_ajaxReq_AliChina_getConnDetail_h_
#define _CmdHandler_ajaxReq_AliChina_getConnDetail_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_AliChina_getConnDetail
     *
     * la GUI ha mandato una richiesta AJAX per conoscere i dettagli della connessione al server alipayChina
     *
        Input:
            command: AliChinaGetConnDetail
            params:
				none

		Output
		    json
		    {
			    ip1: "stringa con la prima parte dell'indirizzo ip del server"
                ip2: ...
                ip3: ...
                ip4: ...
			    port: porta ip del server
			    mid: stringa con il "machine id"
                key: stringa con la chiave crittografica
		    }
     */


    class CmdHandler_ajaxReq_AliChina_getConnDetail : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_AliChina_getConnDetail(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}


        static const char*  getCommandName()                            { return "AliChinaGetConnDetail"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_AliChina_getConnDetail_h_
