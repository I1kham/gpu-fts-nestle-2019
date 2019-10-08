#ifndef _CmdHandler_ajaxReqDBQ_h_
#define _CmdHandler_ajaxReqDBQ_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqDBQ
     *
     * la GUI ha mandato una richiesta AJAX per fare una query su un DB precedentemente aperto con DBC
     *
        Input:
            command: DBQ
            h:  dbHandle (un intero 16 bit > 0)
			sql: la query

        Output
            stringa con tutto il recordset come ritornato da rhea::SQLRst::blobToString()
				oppure
			KO
     */
    class CmdHandler_ajaxReqDBQ : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqDBQ(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params) {}
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) {}

        static const char*  getCommandName()                            { return "DBQ"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqDBQ_h_
