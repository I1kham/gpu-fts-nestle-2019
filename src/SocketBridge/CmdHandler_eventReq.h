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
                                CmdHandler_eventReq (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                                    CmdHandler(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                                {
                                }

		virtual bool			needToPassDownToCPUBridge() const = 0;
		virtual void            passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen) = 0;
		//virtual void			onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;

		virtual void			handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient) 
								{ 
									//le classi derivate devono implementare questa fn SOLO se il messaggio in questione è di quelli che
									//viene gestito direttamente da socketBridge.
									//Si invece si tratta di un msg che deve essere passato al CPUBrige, i metodi da implementare sono passDownRequestToCPUBridge() e
									//onCPUBridgeNotification()
									DBGBREAK;  
								}

    };

    /*********************************************************
     * CmdHandler_eventReqFactory
     *
     *
     */
    class CmdHandler_eventReqFactory
    {
    public:
        static CmdHandler_eventReq* spawnFromSocketClientEventType (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec);
		static CmdHandler_eventReq* spawnFromCPUBridgeEventID (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec);
    };
}//namespace socketbridge

#endif // _CmdHandler_eventReq_h_
