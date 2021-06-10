#ifndef _CmdHandler_ajaxReqFSmkdir_h_
#define _CmdHandler_ajaxReqFSmkdir_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFSmkdir
     *
     * Il client ha mandato una richiesta AJAX per creare una cartella o struttura di cartelle e sottocartelle
     *
        Input:
            command: FSmkdir
            path:	absoluteFolderPath

        Output
			OK
			KO
     */
    class CmdHandler_ajaxReqFSmkdir : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFSmkdir(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSmkdir"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFSmkdir_h_
