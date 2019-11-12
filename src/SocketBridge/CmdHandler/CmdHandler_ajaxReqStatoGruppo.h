#ifndef _CmdHandler_ajaxReqStatoGruppo_h_
#define _CmdHandler_ajaxReqStatoGruppo_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqStatoGruppo
     *
     *
        Input:
            command: getGroupState
            params:  none

		Output
			stringa "1" oppure "0"
				1 vuol dire gruppo collegato, 0 vuol dire gruppo scollegato
		
     */


    class CmdHandler_ajaxReqStatoGruppo : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqStatoGruppo(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getGroupState"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqStatoGruppo_h_
