#ifndef _CmdHandler_ajaxReq_P0x1C_StartModemTest_h_
#define _CmdHandler_ajaxReq_P0x1C_StartModemTest_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x1C_StartModemTest
     *
     * la GUI richiede il valore di tutti i decounterProdotto + i 3 contatori water_filter_dec, coffee_brewer_dec, coffee_ground_dec
     *
        Input:
            command: startModemTest
            params:  none

        Output:
			OK
     */


    class CmdHandler_ajaxReq_P0x1C_StartModemTest : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x1C_StartModemTest(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const													{ return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()																	{ return "startModemTest"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x1C_StartModemTest_h_
