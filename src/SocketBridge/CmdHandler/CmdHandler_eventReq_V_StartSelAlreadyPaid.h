#ifndef _CmdHandler_eventReq_V_StartSelAlreadyPaid_h_
#define _CmdHandler_eventReq_V_StartSelAlreadyPaid_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_V_StartSelAlreadyPaid
     *
     *  Il client richiede che sia erogata la selezione N notificando la CPU che la selezione in questione è già
     *  stata pagata in qualche modo. La CPU deve avviare la selezione bypassando i meccanismi di pagamento
     *  La richiesta viene passata a CPUBridge.
     *  Non sono previste risposte da parte di CPUBridge (ie: CPUBRIDGE_NOTIFY_MAX_ALLOWED)
     */
    class CmdHandler_eventReq_V_StartSelAlreadyPaid : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_startSelectionAlreadyPaid;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

                    CmdHandler_eventReq_V_StartSelAlreadyPaid (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																								{ return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReq_V_StartSelAlreadyPaid_h_
