#ifndef _CmdHandler_eventReq_P0x04_SetDecounter_h_
#define _CmdHandler_eventReq_P0x04_SetDecounter_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_P0x04_SetDecounter
     *
     *	parametri:
			u8	decounter_num
			u16	decounter_value
     *
     */
    class CmdHandler_eventReq_P0x04_SetDecounter : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::setDecounter;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET;

		CmdHandler_eventReq_P0x04_SetDecounter(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);



    };
} // namespace socketbridge

#endif // _CmdHandler_eventReq_P0x04_SetDecounter_h_
