#ifndef _CmdHandler_eventReq_T_VMCDataFileTimestamp_h_
#define _CmdHandler_eventReq_T_VMCDataFileTimestamp_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_T_VMCDataFileTimestamp
     *
     *  Il client vuole conosce il timestamp del DA3 calcolato da CPU
     *  La richiesta viene passata a CPUBridge la quale risponde con una notifica CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP
     *
     */
    class CmdHandler_eventReq_T_VMCDataFileTimestamp : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::reqVMCDataFileTimestamp;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP;

		CmdHandler_eventReq_T_VMCDataFileTimestamp(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReq_T_VMCDataFileTimestamp_h_
