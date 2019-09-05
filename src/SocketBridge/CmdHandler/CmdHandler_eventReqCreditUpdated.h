#ifndef _CmdHandler_eventReqCreditUpdated_h_
#define _CmdHandler_eventReqCreditUpdated_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqCreditUpdated
     *
     *
     *
     */
    class CmdHandler_eventReqCreditUpdated : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_creditUpdated;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED;

                    CmdHandler_eventReqCreditUpdated (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqCreditUpdated_h_
