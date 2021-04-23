#ifndef _CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue_h_
#define _CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue
     *
     *	Chiude a CPU l'attuale lettura sul sensore "Cup"
     *
        Input:
            command: getCupSensorLiveValue
            params:  none

        Output:
			speed: una stringa che rappresenta un numero intero a 16 bit
     */


    class CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getCupSensorLiveValue"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue_h_
