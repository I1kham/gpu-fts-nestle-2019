#ifndef _CmdHandler_ajaxReqDBCloseByPath_h_
#define _CmdHandler_ajaxReqDBCloseByPath_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqDBCloseByPath
     *
     * la GUI ha mandato una richiesta AJAX per chiudere un DB usando il suo path
     *
        Input:
            command: DBClose
            path:  absolutePathToDB

        Output:
			OK

     */
    class CmdHandler_ajaxReqDBCloseByPath : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqDBCloseByPath(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "DBClose"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqDBCloseByPath_h_
