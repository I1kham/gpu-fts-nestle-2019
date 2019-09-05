#ifndef _CmdHandler_eventReq_h_
#define _CmdHandler_eventReq_h_
#include "CmdHandler.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq
     *
     * Classe di base per la gestione delle richieste "event"
     */
    class CmdHandler_eventReq : public CmdHandler
    {
    public:
                                CmdHandler_eventReq (const HSokServerClient &h, u16 handlerID, u64 dieAfterHowManyMSec) :
                                    CmdHandler(h, handlerID, dieAfterHowManyMSec)
                                {
                                }

        virtual void            handleRequest (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen) = 0;
		//virtual void			handleAnswer (socketbridge::Server *server, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
    };

    /*********************************************************
     * CmdHandler_eventReqFactory
     *
     *
     */
    class CmdHandler_eventReqFactory
    {
    public:
        static CmdHandler_eventReq* spawnFromSocketClientEventType (rhea::Allocator *allocator, const HSokServerClient &h, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec);
		static CmdHandler_eventReq* spawnFromCPUBridgeEventID (rhea::Allocator *allocator, const HSokServerClient &h, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec);
    };
}//namespace socketbridge

#endif // _CmdHandler_eventReq_h_
