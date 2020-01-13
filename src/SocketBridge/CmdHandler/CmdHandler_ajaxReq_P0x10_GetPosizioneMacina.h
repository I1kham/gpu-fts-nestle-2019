#ifndef _CmdHandler_ajaxReq_P0x10_GetPosizioneMacina_h_
#define _CmdHandler_ajaxReq_P0x10_GetPosizioneMacina_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x10_GetPosizioneMacina
     *
     * la GUI ha mandato una richiesta AJAX per conoscere la posizione della macina (1 o 2)
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


    class CmdHandler_ajaxReq_P0x10_GetPosizioneMacina : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x10_GetPosizioneMacina(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getPosMacina"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x10_GetPosizioneMacina_h_
