#ifndef _CmdHandler_ajaxReqSetCalibFactor_h_
#define _CmdHandler_ajaxReqSetCalibFactor_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqSetCalibFactor
     *
     *
        Input:
            command: setFattoreCalib
            params:
				m: motor, da 1 a 13 vedi enum cpubridge::eCPUProgrammingCommand_motor
				v: valore a cui settare (16bit)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReqSetCalibFactor : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqSetCalibFactor(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "setFattoreCalib"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqSetCalibFactor_h_
