#ifndef _CmdHandler_ajaxReq_P0x0C_AttivazioneMotore_h_
#define _CmdHandler_ajaxReq_P0x0C_AttivazioneMotore_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x0C_AttivazioneMotore
     *
     *
        Input:
            command: runMotor
            params:
				m: motor, da 1 a 13 vedi enum cpubridge::eCPUProgrammingCommand_motor
				d: durata in dSec (intero)
				n: numero di ripetuzioni (intero)
				p: pausa tra una rip e l'altra (dsec)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReq_P0x0C_AttivazioneMotore : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x0C_AttivazioneMotore(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "runMotor"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqAttivazioneMotore_h_
