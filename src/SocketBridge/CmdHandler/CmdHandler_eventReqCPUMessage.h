#ifndef _CmdHandler_eventReqCPUMessage_h_
#define _CmdHandler_eventReqCPUMessage_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqCPUMessage
     *
     *  Il client richiede l'attuale messaggio di CPU
     *  La richiesta viene passata a CPUBridge la quale risponde con una notifica CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE
     *  La stessa notifica "CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE" può essere generata spontaneamente da CPUBridge in ogni momento, non
     *  necessariamente a seguito di una richiesta.
     *  Quando SocketBridge riceve una notifica CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, se questà è la risposta ad una precisa richiesta, allora
     *  invia la risposta al solo client che ne ha fatto richiesta.
     *  Se la notifica invece è stata generata spontaneamente da CPUBridge, SocketBridge invia l'evento a tutti i client connessi
     *
     */
    class CmdHandler_eventReqCPUMessage : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT	= eEventType::cpuMessage;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE;

                    CmdHandler_eventReqCPUMessage (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqCPUMessage_h_
