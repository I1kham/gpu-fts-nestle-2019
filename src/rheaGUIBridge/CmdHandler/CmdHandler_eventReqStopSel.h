#ifndef _CmdHandler_eventReqStopSel_h_
#define _CmdHandler_eventReqStopSel_h_
#include "CmdHandler_eventReq.h"



namespace guibridge
{
    /*********************************************************
     * CmdHandler_eventReqStopSel
     *
     *
     *
     */
    class CmdHandler_eventReqStopSel : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE = eEventType_stopSelection;

                    CmdHandler_eventReqStopSel (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen);
        void        handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU);
    };
} // namespace guibridge
#endif // _CmdHandler_eventReqStopSel_h_
