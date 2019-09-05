#ifndef _CmdHandler_eventReqStartSel_h_
#define _CmdHandler_eventReqStartSel_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqStartSel
     *
     *
     *
     */
    class CmdHandler_eventReqStartSel : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_startSelection;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

                    CmdHandler_eventReqStartSel (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqStartSel_h_
