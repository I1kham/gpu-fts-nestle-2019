#ifndef _CmdHandler_ajaxReq_snack_0x03_stato_h_
#define _CmdHandler_ajaxReq_snack_0x03_stato_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_snack_0x03_stato
     *
     *
        Input:
            command: snackGetState
            params:  none

		Output
			json
            {
                state: 0|1
                n: numero di selezioni
                avail: stringa di n char (dove n=numero selezioni):
                        il char '0' indica che la sel non è disponibile
                        il char '1' indica che è disponibile
        }
     */


    class CmdHandler_ajaxReq_snack_0x03_stato : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_snack_0x03_stato(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "snackGetState"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_snack_0x03_stato_h_
