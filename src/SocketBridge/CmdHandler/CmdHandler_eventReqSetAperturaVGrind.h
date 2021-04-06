#ifndef _CmdHandler_eventReqSetAperturaVGrind_h_
#define _CmdHandler_eventReqSetAperturaVGrind_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSetAperturaVGrind
     *
     *  Il client vuole aprire/chiudere il varigrind e portarlo in una nuova posizione.
     *  La richiesta viene passata a CPUBridge la quale non risponderà direttamente con alcuna notifica specifica, ma bensì
     *  entra in uno stato specifico fino al termine della regolazione.
     *  Durante la regolazione della macina, CPU passa nello stato 102 (e notifica con CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED).
     *  Al termine della regolazione, CPU torna nello stato disponibile (e notifica con CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED)
     *
		params:
			u8	macina 1 o 2
			u16	posizione target
     *
     */
    class CmdHandler_eventReqSetAperturaVGrind : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::setAperturaVGrind;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

		CmdHandler_eventReqSetAperturaVGrind(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																								{ return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqSetAperturaVGrind_h_
