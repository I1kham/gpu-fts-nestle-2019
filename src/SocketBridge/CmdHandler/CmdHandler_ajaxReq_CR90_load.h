#ifndef _CmdHandler_ajaxReq_CR90_load_h_
#define _CmdHandler_ajaxReq_CR90_load_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_CR90_load
     *
     * la GUI ha mandato una richiesta AJAX per settare uno dei decounter prodotti
     *
        Input:
            command: CR90-load
            params: none

		Output
			stringa di interi separati da ","
     */


    class CmdHandler_ajaxReq_CR90_load : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_CR90_load(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const           { return false; }
        void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "CR90-load"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_CR90_load_h_
