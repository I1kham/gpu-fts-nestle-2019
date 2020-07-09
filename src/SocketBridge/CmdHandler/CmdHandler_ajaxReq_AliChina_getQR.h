#ifndef _CmdHandler_ajaxReq_AliChina_getQR_h_
#define _CmdHandler_ajaxReq_AliChina_getQR_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_AliChina_getQR
     *
     * la GUI ha mandato una richiesta AJAX per conoscere il QR code per il pagamento tramite AlipayChina
     *
        Input:
            command: AliChinaGetQR
            params:
				name: nome della selezione
				selNum: numero della selezione
                price: stringa col prezzo

		Output
			KO
            oppure
            una URL da embeddare nel qr
     */


    class CmdHandler_ajaxReq_AliChina_getQR : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_AliChina_getQR(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}


        static const char*  getCommandName()                            { return "AliChinaGetQR"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_AliChina_getQR_h_
