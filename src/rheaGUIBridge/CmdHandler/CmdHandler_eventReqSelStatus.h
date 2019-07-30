#ifndef _CmdHandler_eventReqSelStatus_h_
#define _CmdHandler_eventReqSelStatus_h_
#include "CmdHandler_eventReq.h"



namespace guibridge
{
    /*********************************************************
     * CmdHandler_eventReqSelFinished
     *
     *
     *
     */
    class CmdHandler_eventReqSelStatus : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE = eEventType_selectionRequestStatus;

                    CmdHandler_eventReqSelStatus (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen);
        void        handleAnswerToGUI (rhea::ProtocolServer *server, const u8 *dataFromGPU);
    };
} // namespace guibridge

#endif // _CmdHandler_eventReqSelStatus_h_
