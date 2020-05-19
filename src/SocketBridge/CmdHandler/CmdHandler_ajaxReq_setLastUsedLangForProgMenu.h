#ifndef _CmdHandler_ajaxReq_setLastUsedLangForProgMenu_h_
#define _CmdHandler_ajaxReq_setLastUsedLangForProgMenu_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_setLastUsedLangForProgMenu
     *
     *  Il client vuole conoscere l'ultimo linguaggio utilizzato nel menu di programmazione.
        Input:
            command: setLastUsedLangPrgMenu
            params:
				lang: stringa composta da 2 char maiuscoli indicanti la lingua (es: GB IT FR)

		Output
			OK
     */


    class CmdHandler_ajaxReq_setLastUsedLangForProgMenu : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_setLastUsedLangForProgMenu(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
		void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "setLastUsedLangPrgMenu"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_setLastUsedLangForProgMenu_h_
