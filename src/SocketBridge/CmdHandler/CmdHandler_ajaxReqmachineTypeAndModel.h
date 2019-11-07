#ifndef _CmdHandler_ajaxReqmachineTypeAndModel_h_
#define _CmdHandler_ajaxReqmachineTypeAndModel_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqmachineTypeAndModel
     *
     * la GUI ha mandato una richiesta AJAX per conosce il machine type (espresso o instant) e il model (un numero che indentifica il modello, tipo BL o Fusion)
     *
        Input:
            command: getMachineTypeAndModel
            params:  none

        Output
		json
		{
			mType: numero 1 se espresso, 2 se instant
			mModel: numero intero 8bit
		}
     */


    class CmdHandler_ajaxReqmachineTypeAndModel : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqmachineTypeAndModel(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getMachineTypeAndModel"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqmachineTypeAndModel_h_
