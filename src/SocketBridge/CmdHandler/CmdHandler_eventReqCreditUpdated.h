#ifndef _CmdHandler_eventReqCreditUpdated_h_
#define _CmdHandler_eventReqCreditUpdated_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqCreditUpdated
     *
     *  Il client richiede il credito attuale di CPU
     *  La richiesta viene passata a CPUBridge la quale risponde con una notifica CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED.
     *  La stessa notifica "CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED" può essere generata spontaneamente da CPUBridge in ogni momento, non
     *  necessariamente a seguito di una richiesta.
     *  Quando SocketBridge riceve una notifica CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED, se questà è la risposta ad una precisa richiesta, allora
     *  invia la risposta al solo client che ne ha fatto richiesta.
     *  Se la notifica invece è stata generata spontaneamente da CPUBridge, SocketBridge invia l'evento a tutti i client connessi
     *
     */
    class CmdHandler_eventReqCreditUpdated : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::creditUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED;

                    CmdHandler_eventReqCreditUpdated (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqCreditUpdated_h_
