#ifndef _CmdHandler_eventReqDataAudit_h_
#define _CmdHandler_eventReqDataAudit_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqDataAudit
     *
     *
     *
     */
    class CmdHandler_eventReqDataAudit : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqDataAudit;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS;

		CmdHandler_eventReqDataAudit(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqDataAudit_h_
