#ifndef _CmdHandler_ajaxReqGetPosizioneMacina_h_
#define _CmdHandler_ajaxReqGetPosizioneMacina_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetPosizioneMacina
     *
     * la GUI ha mandato una richiesta AJAX per settare uno dei decounter prodotti
     *
        Input:
            command: getPosMacina
            params:
				m: macina (1=macina1, 2=macina2)

		Output
			json
			{
				m: macina (1=macina1, 2=macina2)
				v: intero, posizione della macina
			}
     */


    class CmdHandler_ajaxReqGetPosizioneMacina : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqGetPosizioneMacina(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getPosMacina"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetPosizioneMacina_h_
