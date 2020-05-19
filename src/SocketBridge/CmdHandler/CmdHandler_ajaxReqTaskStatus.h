#ifndef _CmdHandler_ajaxReqTaskStatus_
#define _CmdHandler_ajaxReqTaskStatus_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqTaskStatus
     *
     * Il client ha mandato una richiesta AJAX per conoscere lo stato del task i-esimo
     *
        Input:
            command:	taskStatus
			id:			uid del task ritornato da taskSpanw

        Output:
			json
			{
				status:		0 | 1	=> 0=finished, 1=running
				msg:		stringa con l'ultimo messaggio di stato riportato dal task
			}
     */
    class CmdHandler_ajaxReqTaskStatus : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqTaskStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "taskStatus"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqTaskStatus_
