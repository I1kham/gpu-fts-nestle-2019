#ifndef _CmdHandler_eventReqWriteLocalVMCDataFile_h_
#define _CmdHandler_eventReqWriteLocalVMCDataFile_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqWriteLocalVMCDataFile
     *
     *
     *
     */
    class CmdHandler_eventReqWriteLocalVMCDataFile : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqWriteLocalVMCDataFile;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS;
		
		CmdHandler_eventReqWriteLocalVMCDataFile(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqWriteLocalVMCDataFile_h_
