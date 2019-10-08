#ifndef _CmdHandler_ajaxReq_h_
#define _CmdHandler_ajaxReq_h_
#include "CmdHandler.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq
     *
     * Classe di base per la gestione delle richieste "ajax"
     */
    class CmdHandler_ajaxReq : public CmdHandler
    {
    public:
                                CmdHandler_ajaxReq (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                    CmdHandler(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                                {
                                    this->ajaxRequestID = ajaxRequestID;
                                }

        virtual void            passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params) = 0;
		//virtual void			onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
		//virtual bool			needToPassDownToCPUBridge() const = 0;

		virtual void			handleRequestFromSocketBridge (socketbridge::Server *server, HSokServerClient &hClient, const char *params)
								{
									//le classi derivate devono implementare questa fn SOLO se il messaggio in questione � di quelli che
									//viene gestito direttamente da socketBridge.
									//Si invece si tratta di un msg che deve essere passato al CPUBrige, i metodi da implementare sono passDownRequestToCPUBridge() e
									//onCPUBridgeNotification()
									DBGBREAK;
								}

    protected:

    protected:
        u8                      ajaxRequestID;
    };






    /*********************************************************
     * CmdHandler_ajaxReqFactory
     *
     *
     */
    class CmdHandler_ajaxReqFactory
    {
    public:
        static CmdHandler_ajaxReq* spawn (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec,
                                                   const char **out_params);
    };
} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_h_
