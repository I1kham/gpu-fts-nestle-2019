#ifndef _CmdHandler_eventReqStartSel_h_
#define _CmdHandler_eventReqStartSel_h_
#include "CmdHandler_eventReq.h"



namespace guibridge
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
        static const eEventType EVENT_TYPE = eEventType_startSelection;

                    CmdHandler_eventReqStartSel (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen);
        void        handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU);
    };
} // namespace guibridge
#endif // _CmdHandler_eventReqStartSel_h_
