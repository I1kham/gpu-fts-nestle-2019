#ifndef _CmdHandler_ajaxReqIsManualInstalled_
#define _CmdHandler_ajaxReqIsManualInstalled_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqIsManualInstalled
     *
     * Il client ha mandato una richiesta AJAX per sapere se localmente esiste un manuale per il menu di prog
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
								//ritorna false in quanto questo comando non deve essere propagato a CPUBridge

		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) { }
								//vuota in quanto questo comando non viene propagato a CPUBrdige

        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) { }
								//vuota in quanto questo comando non viene propagato a CPUBrdige

        static const char*  getCommandName()                            { return "isManInstalled"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqIsManualInstalled_
