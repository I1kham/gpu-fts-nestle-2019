#ifndef _CmdHandler_ajaxReq_P0x03_SanWashStatus_h_
#define _CmdHandler_ajaxReq_P0x03_SanWashStatus_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x03_SanWashStatus
     *
     * la GUI ha mandato una richiesta AJAX per conosce lo stato di avanzamento del lavaggio sanitario
     *
        Input:
            command: sanWashStatus
            params:  none

        Output
        json
        {
            fase: numero intero 8bit che indica la fase del lavaggio
            btn1: numero intero 8bit che indica se la CPU è in attesa della pressione di uno specifico btn (0==nessun bottone)
            btn2: numero intero 8bit che indica se la CPU è in attesa della pressione di uno specifico btn (0==nessun bottone)
        }
     */


    class CmdHandler_ajaxReq_P0x03_SanWashStatus : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x03_SanWashStatus(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "sanWashStatus"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x03_SanWashStatus_h_
