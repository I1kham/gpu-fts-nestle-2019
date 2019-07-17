#ifndef _CmdHandler_eventReqSelPrices_h_
#define _CmdHandler_eventReqSelPrices_h_
#include "CmdHandler_eventReq.h"



namespace guibridge
{
    /*********************************************************
     * CmdHandler_eventReqSelPrices
     *
     *
     *
     */
    class CmdHandler_eventReqSelPrices : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE = eEventType_selectionPricesUpdated;

                    CmdHandler_eventReqSelPrices (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(h, handlerID, dieAfterHowManyMSec)
                    {  }

        void        handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen);
        void        handleAnswerToGUI (WebsocketServer *server, const u8 *dataFromGPU);
    };
} // namespace guibridge

#endif // _CmdHandler_eventReqSelPrices_h_
