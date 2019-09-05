#ifndef _CmdHandler_eventReqSelPrices_h_
#define _CmdHandler_eventReqSelPrices_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSelPrices
     *
     *
     *
     */
    class CmdHandler_eventReqSelPrices : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_selectionPricesUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED;

                    CmdHandler_eventReqSelPrices (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge);



    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqSelPrices_h_
