#ifndef _CmdHandler_eventReqSelStatus_h_
#define _CmdHandler_eventReqSelStatus_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqSelStatus
     *
     *  Il client vuole conoscere lo stato della erogazione corrente
     *  La richiesta viene passata a CPUBridge la quale risponderà con una notifica CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS
     *
     */
    class CmdHandler_eventReqSelStatus : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_selectionRequestStatus;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS;

                    CmdHandler_eventReqSelStatus (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const { return true; }
		void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqSelStatus_h_
