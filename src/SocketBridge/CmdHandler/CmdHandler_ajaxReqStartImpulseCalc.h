#ifndef _CmdHandler_ajaxReqStartImpulseCalc_h_
#define _CmdHandler_ajaxReqStartImpulseCalc_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqStartImpulseCalc
     *
     * la GUI ha mandato una richiesta AJAX per settare uno dei decounter prodotti
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


    class CmdHandler_ajaxReqStartImpulseCalc : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqStartImpulseCalc(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "startImpulseCalc"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqStartImpulseCalc_h_
