#ifndef _CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue_h_
#define _CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue
     *
     *	A seguito di un comando [CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest], usare questo cmd per recuperare il valore
	 *	medio del sensore di rotazione della macina
     *
        Input:
            command: getLastGrndSpeedValue
            params:  none

        Output:
			speed: una stringa che rappresenta un numero intero
     */


    class CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getLastGrndSpeedValue"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue_h_
