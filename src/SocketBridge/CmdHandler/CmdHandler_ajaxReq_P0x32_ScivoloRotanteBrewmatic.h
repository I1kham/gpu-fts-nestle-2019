#ifndef _CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic_h_
#define _CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic
     *
     *
        Input:
            command:  
            params:
				perc: intero 0-100 che indica la % di PWM da applicare (0 = stop)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "scivoloBrewmatic"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqAttivazioneMotore_h_
