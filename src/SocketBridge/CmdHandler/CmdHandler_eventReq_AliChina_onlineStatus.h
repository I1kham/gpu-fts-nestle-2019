#ifndef _CmdHandler_eventReq_AliChina_onlineStatus_h_
#define _CmdHandler_eventReq_AliChina_onlineStatus_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_AliChina_onlineStatus
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
    class CmdHandler_eventReq_AliChina_onlineStatus : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT	= eEventType_AlipayChina_onlineStatusChanged;
		static const u16		EVENT_ID_FROM_CPUBRIDGE         = CPUBRIDGE_NOTIFY_MAX_ALLOWED;

                    CmdHandler_eventReq_AliChina_onlineStatus (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

        bool		needToPassDownToCPUBridge() const                                                           { return false; }
        void		handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient);

        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM) { }
        void		onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) { }

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReq_AliChina_onlineStatus_h_
