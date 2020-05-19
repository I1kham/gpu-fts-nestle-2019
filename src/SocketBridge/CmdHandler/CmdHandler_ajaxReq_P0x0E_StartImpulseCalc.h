#ifndef _CmdHandler_ajaxReq_P0x0E_StartImpulseCalc_h_
#define _CmdHandler_ajaxReq_P0x0E_StartImpulseCalc_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x0E_StartImpulseCalc
     *
     * GUI chiede di avviare la procedura di calcolo degli impulsi. 
	 * A seguire, GUI pollerà usando con il comando 0x0E per conoscere lo stato di avanzamento dell'operazione
     *
        Input:
            command: startImpulseCalc
            params:
				m: macina (11=macina1, 12=macina2)
				v: valore della pesata in dGrammi

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReq_P0x0E_StartImpulseCalc : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x0E_StartImpulseCalc(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "startImpulseCalc"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x0E_StartImpulseCalc_h_
