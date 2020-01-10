#ifndef _CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo_h_
#define _CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo
     *
     * la GUI richiede il valore del flusso e della posizione dell'apertura della macina relative all'ultima selezione effettuata
     *
        Input:
            command: getLastFluxInfo
            params:  none

        Output:
			json
			{
				flux: numero
				grinderPos: numero
			}
     */


    class CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getLastFluxInfo"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo_h_
