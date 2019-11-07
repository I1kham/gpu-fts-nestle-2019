#ifndef _CmdHandler_ajaxReqSetDecounter_h_
#define _CmdHandler_ajaxReqSetDecounter_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqSetDecounter
     *
     * la GUI ha mandato una richiesta AJAX per settare uno dei decounter prodotti
     *
        Input:
            command: setDecounterProd
            params:
				d: 1..13 vedi enum cpubridge::eCPUProgrammingCommand_decounter
				v: valore a cui resettare (16bit)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReqSetDecounter : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqSetDecounter(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "setDecounter"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqSetDecounter_h_
