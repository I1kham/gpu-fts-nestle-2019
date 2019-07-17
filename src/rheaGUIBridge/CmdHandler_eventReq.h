#ifndef _CmdHandler_eventReq_h_
#define _CmdHandler_eventReq_h_
#include "CmdHandler.h"


namespace guibridge
{
    /*********************************************************
     * CmdHandler_eventReq
     *
     * Classe di base per la gestione delle richieste "event"
     */
    class CmdHandler_eventReq : public CmdHandler
    {
    public:
                                CmdHandler_eventReq (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                                    CmdHandler(h, handlerID, dieAfterHowManyMSec)
                                {
                                }

        virtual void            handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const u8 *payload, u16 payloadLen) = 0;
    };

    /*********************************************************
     * CmdHandler_eventReqFactory
     *
     *
     */
    class CmdHandler_eventReqFactory
    {
    public:
        static CmdHandler_eventReq* spawn (rhea::Allocator *allocator, const HWebsokClient &h, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec);
    };
}//namespace guibridge

#endif // _CmdHandler_eventReq_h_
