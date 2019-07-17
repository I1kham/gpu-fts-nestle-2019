#ifndef _CmdHandler_ajaxReq_h_
#define _CmdHandler_ajaxReq_h_
#include "CmdHandler.h"


namespace guibridge
{
    /*********************************************************
     * CmdHandler_ajaxReq
     *
     * Classe di base per la gestione delle richieste "ajax"
     */
    class CmdHandler_ajaxReq : public CmdHandler
    {
    public:
                                CmdHandler_ajaxReq (const HWebsokClient &h, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                    CmdHandler(h, handlerID, dieAfterHowManyMSec)
                                {
                                    this->ajaxRequestID = ajaxRequestID;
                                }

        virtual void            handleRequestFromGUI (const HThreadMsgW hQMessageToWebserver, const char *params) = 0;

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
        static CmdHandler_ajaxReq* spawn (rhea::Allocator *allocator, const HWebsokClient &h, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec,
                                                   const char **out_params);
    };
} // namespace guibridge

#endif // _CmdHandler_ajaxReq_h_
