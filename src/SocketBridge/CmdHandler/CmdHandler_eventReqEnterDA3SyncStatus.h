#ifndef _CmdHandler_eventReqEnterDA3SyncStatus_h_
#define _CmdHandler_eventReqEnterDA3SyncStatus_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqEnterDA3SyncStatus
     *
     *  Il client vuole che la CPU entri nello stato di sincronizzazione del DA3
     *  La richiesta viene passata a CPUBridge dalla quale non ci si aspetta alcuna risposta diretta.
     *  CPUBridge entrerà nello stato DA3Sync() e le usuali notifiche spontanee verranno generate (es: tutte le notifiche relative
     *  al cambio di stato della CPU e le eventuali notifiche di sincronizzazione in corso del DA3)
     */
    class CmdHandler_eventReqEnterDA3SyncStatus : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::enterDA3Sync;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

                    CmdHandler_eventReqEnterDA3SyncStatus (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqEnterDA3SyncStatus_h_
