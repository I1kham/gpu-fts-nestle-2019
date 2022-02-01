#ifndef _CmdHandler_ajaxReq_CR90_save_h_
#define _CmdHandler_ajaxReq_CR90_save_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_CR90_save
     *
     * la GUI ha mandato una richiesta AJAX per salvare il valore di una delle celle
     *
        Input:
            command: CR90-load
            params:
				index:  indice della cella, da 0 a N
				valuve: intero

		Output
			OK
     */


    class CmdHandler_ajaxReq_CR90_save : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_CR90_save(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const           { return false; }
        void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "CR90-save"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_CR90_save_h_
