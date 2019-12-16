#ifndef _CmdHandler_ajaxReqIsManualInstalled_
#define _CmdHandler_ajaxReqIsManualInstalled_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqIsManualInstalled
     *
     * la GUI ha mandato una richiesta AJAX per conosce l'elenco dei drive di sistema
     *
        Input:
            command: isManInstalled

        Output
			manualFolderName:	stringa con il nome della cartella che contiene il manuale
								oppure
								KO
        }
     */
    class CmdHandler_ajaxReqIsManualInstalled : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqIsManualInstalled(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "isManInstalled"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqIsManualInstalled_
