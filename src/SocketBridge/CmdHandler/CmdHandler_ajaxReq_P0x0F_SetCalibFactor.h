#ifndef _CmdHandler_ajaxReq_P0x0F_SetCalibFactor_h_
#define _CmdHandler_ajaxReq_P0x0F_SetCalibFactor_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x0F_SetCalibFactor
     *
     *
        Input:
            command: setFattoreCalib
            params:
				m: motor, da 1 a 20 vedi enum cpubridge::eCPUProg_motor
				v: valore a cui settare (16bit)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReq_P0x0F_SetCalibFactor : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x0F_SetCalibFactor(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "setFattoreCalib"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x0F_SetCalibFactor_h_
