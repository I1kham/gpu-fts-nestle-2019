#ifndef _CmdHandler_ajaxReqGetLastInstalledGUIFilename_
#define _CmdHandler_ajaxReqGetLastInstalledGUIFilename_
#include "../CmdHandler_ajaxReq.h"

namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetLastInstalledGUIFilename
     *
     * Il client ha mandato una richiesta AJAX per il nome del file della GUI installata
     *
        Input:
            command: ajaxReqGetLastInstalledGUIFilename

        Output
			filename:	stringa con il nome del file della GUI installato
								oppure
								KO
        }
     */
    class CmdHandler_ajaxReqGetLastInstalledGUIFilename : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqGetLastInstalledGUIFilename(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "getLastInstalledGUIFilename"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqGetLastInstalledGUIFilename_
