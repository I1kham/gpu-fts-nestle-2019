#ifndef _CmdHandler_eventReqCPUMessage_h_
#define _CmdHandler_eventReqCPUMessage_h_
#include "CmdHandler_eventReq.h"



namespace guibridge
{
    /*********************************************************
     * CmdHandler_eventReqCPUMessage
     *
     *
     *
     */
    class CmdHandler_eventReqCPUMessage : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE = eEventType_cpuMessage;

                    CmdHandler_eventReqCPUMessage (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen);
        void        handleAnswerToGUI (rhea::ProtocolServer *server, const u8 *dataFromGPU);
    };
} // namespace guibridge

#endif // _CmdHandler_eventReqCPUMessage_h_
