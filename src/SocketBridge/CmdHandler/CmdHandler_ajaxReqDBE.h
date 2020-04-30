#ifndef _CmdHandler_ajaxReqDBE_h_
#define _CmdHandler_ajaxReqDBE_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqDBE
     *
     * Il client ha mandato una richiesta AJAX per fare una exec su un DB precedentemente aperto con DBC
     *
        Input:
            command: DBE
            h:  dbHandle (un intero 16 bit > 0)
			sql: la query

        Output
            OK
			KO
     */
    class CmdHandler_ajaxReqDBE : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqDBE(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "DBE"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqDBE_h_
