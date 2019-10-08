#ifndef _CmdHandler_ajaxReqDBC_h_
#define _CmdHandler_ajaxReqDBC_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqDBC
     *
     * la GUI ha mandato una richiesta AJAX per conosce collegarsi ad un db locale
     *
        Input:
            command: DBC
            path:  absolutePathToDB

        Output
        json
        {
            handle: un intero 16 bit sempre > 0 che identifica univocamente la connessione al DB
					se ritorna 0 vuol dire che non è stato possibile aprire il DB
        }
     */
    class CmdHandler_ajaxReqDBC : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqDBC(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params) {}
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) {}

        static const char*  getCommandName()                            { return "DBC"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqDBC_h_
