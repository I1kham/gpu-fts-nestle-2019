#ifndef _CmdHandler_ajaxReq_P0x2A_stopJug_h_
#define _CmdHandler_ajaxReq_P0x2A_stopJug_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x2A_stopJug
     *
     *  GPU informa CPU che è necessario interrompere l'attuale selezione (con JUG) in corso. 
     *  CPU attende fino al completamente dell'attuale ripetizione e poi termina la selezione in toto.
        Input:
            command: stopJug
            params: none

		Output
			OK
            KO
     */


    class CmdHandler_ajaxReq_P0x2A_stopJug : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x2A_stopJug(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "stopJug"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x2A_stopJug_h_
