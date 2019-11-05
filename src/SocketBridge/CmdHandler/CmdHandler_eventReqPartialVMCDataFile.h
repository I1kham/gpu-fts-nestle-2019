#ifndef _CmdHandler_eventReqPartialVMCDataFile_h_
#define _CmdHandler_eventReqPartialVMCDataFile_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqPartialVMCDataFile
     *
     *
     *
     */
    class CmdHandler_eventReqPartialVMCDataFile : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_cpuWritePartialVMCDataFile;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE;
		
				CmdHandler_eventReqPartialVMCDataFile(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqPartialVMCDataFile_h_
