#ifndef _CmdHandler_ajaxReq_h_
#define _CmdHandler_ajaxReq_h_
#include "CmdHandler.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq
     *
     *  ATTENZIONE: Questo è un copia/incolla dello stesso file nel progetto socketBridge.
                Fare riferimento all'originale per le info
     */
    class CmdHandler_ajaxReq : public CmdHandler
    {
    public:
                                CmdHandler_ajaxReq (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                    CmdHandler(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                                {
                                    this->ajaxRequestID = ajaxRequestID;
                                }

		//virtual bool		    needForwardToGPU() const = 0;
        //virtual void		    onGPUNotification (rasPI::socketListener::Core *core, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;

        virtual void            fwdRequestToGPU (const u8 *params) = 0;

        virtual void			handleRequestCore (rasPI::socketListener::Core *core UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const u8 *params UNUSED_PARAM)
								{
									//le classi derivate devono implementare questa fn SOLO se il messaggio in questione è di quelli che
									//viene gestito direttamente da socketBridge senza passare per CPUBridge (es: CmdHandler_eventReqClientList che chiede una lista dei client
									//connessi alla socket).
									//Se invece si tratta di un msg che deve essere passato al CPUBrige, i metodi da implementare sono passDownRequestToCPUBridge() e
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
                                                   const u8 **out_params);
    };
} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_h_
