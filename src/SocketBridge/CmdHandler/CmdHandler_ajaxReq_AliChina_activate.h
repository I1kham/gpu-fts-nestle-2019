#ifndef _CmdHandler_ajaxReq_AliChina_activate_h_
#define _CmdHandler_ajaxReq_AliChina_activate_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_AliChina_activate
     *
     *  la GUI ha mandato una richiesta AJAX per abilitare il servizio AlipayChina
     *  Se il servizio fosse già attivo, non succede nulla, la risposta rimane 'OK'
     *
     *  Dopo che il servizio è stato attivato, è possibile usare AliChinaIsOnline per conoscere la disponibilità del servizio
     *
        Input:
            command: AliChinaActivate
            params:
				none

		Output
			OK
     */


    class CmdHandler_ajaxReq_AliChina_activate : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_AliChina_activate(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}


        static const char*  getCommandName()                            { return "AliChinaActivate"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_AliChina_activate_h_
