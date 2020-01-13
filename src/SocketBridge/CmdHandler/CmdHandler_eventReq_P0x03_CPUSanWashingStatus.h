#ifndef _CmdHandler_eventReq_P0x03_CPUSanWashingStatus_h_
#define _CmdHandler_eventReq_P0x03_CPUSanWashingStatus_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_P0x03_CPUSanWashingStatus
     *
     *
     *
     */
    class CmdHandler_eventReq_P0x03_CPUSanWashingStatus : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_cpuSanWashingStatus;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS;

		CmdHandler_eventReq_P0x03_CPUSanWashingStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																									{ return true; }
		void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge

#endif // _CmdHandler_eventReq_P0x03_CPUSanWashingStatus_h_
