#ifndef _CmdHandler_ajaxReq_P0x0B_StatoGruppo_h_
#define _CmdHandler_ajaxReq_P0x0B_StatoGruppo_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x0B_StatoGruppo
     *
     *
        Input:
            command: getGroupState
            params:  none

		Output
			stringa "1" oppure "0"
				1 vuol dire gruppo collegato, 0 vuol dire gruppo scollegato
		
     */


    class CmdHandler_ajaxReq_P0x0B_StatoGruppo : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x0B_StatoGruppo(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getGroupState"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x0B_StatoGruppo_h_
