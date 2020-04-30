#ifndef _CmdHandler_eventReqSelPrices_h_
#define _CmdHandler_eventReqSelPrices_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSelPrices
     *
     *  Il client vuole conoscere i prezzi delle selezioni.
     *  La richiesta viene passata a CPUBridge la quale risponderà con una notifica CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED
     *
     */
    class CmdHandler_eventReqSelPrices : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_selectionPricesUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED;

                    CmdHandler_eventReqSelPrices (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);



    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqSelPrices_h_
