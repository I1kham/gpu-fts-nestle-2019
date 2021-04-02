#ifndef _CmdHandler_ajaxReqGetDA3info_
#define _CmdHandler_ajaxReqGetDA3info_
#include "../CmdHandler_ajaxReq.h"

namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetDA3info
     *
     * Il client ha mandato una richiesta AJAX per il nome del file della GUI installata
     *
        Input:
            command: getDA3info

        Output
			filename: json structure
                { 
                    filename : nome del file oppure '???' se non trovato,
                    lastUpdate : data dell'ultimo aggiornamento formato stringa 'YYYY/MM/DD hh:mm:ss' oppure '???' se non trovato
                }
     */
    class CmdHandler_ajaxReqGetDA3info : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqGetDA3info(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "getDA3info"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqGetDA3info_
