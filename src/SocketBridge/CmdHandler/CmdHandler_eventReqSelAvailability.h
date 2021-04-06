#ifndef _CmdHandler_eventRequestedFromGUI_h_
#define _CmdHandler_eventRequestedFromGUI_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSelAvailability
     *
     *  Il client vuole conoscere lo stato di disponibilità delle selezioni.
     *  La richiesta viene passata a CPUBridge la quale risponderà con una notifica CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED
     *
     */
    class CmdHandler_eventReqSelAvailability : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::selectionAvailabilityUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED;

                    CmdHandler_eventReqSelAvailability (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);


    };
} // namespace socketbridge
#endif // CmdHandler_eventRequestedFromGUI_h_
