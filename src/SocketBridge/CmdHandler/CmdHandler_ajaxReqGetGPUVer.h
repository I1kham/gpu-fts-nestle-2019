#ifndef _CmdHandler_ajaxReqGetGPUVer_
#define _CmdHandler_ajaxReqGetGPUVer_
#include "../CmdHandler_ajaxReq.h"

namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetGPUVer
     *
     * Il client ha mandato una richiesta AJAX per sapere la versione corrente della GPU
     *
        Input:
            command: getGPUVer

        Output
			GPUver:	stringa con la versione corrente del sw di GPU
					    oppure
					???
     */
    class CmdHandler_ajaxReqGetGPUVer : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqGetGPUVer(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "getGPUVer"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqGetGPUVer_
