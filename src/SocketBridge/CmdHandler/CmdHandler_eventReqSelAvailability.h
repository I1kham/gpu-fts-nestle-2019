#ifndef _CmdHandler_eventRequestedFromGUI_h_
#define _CmdHandler_eventRequestedFromGUI_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSelAvailability
     *
     *
     *
     */
    class CmdHandler_eventReqSelAvailability : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_selectionAvailabilityUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED;

                    CmdHandler_eventReqSelAvailability (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge);


    };
} // namespace socketbridge
#endif // CmdHandler_eventRequestedFromGUI_h_
