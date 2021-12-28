#ifndef _CmdHandler_ajaxReqMachineTypeAndModel_h_
#define _CmdHandler_ajaxReqMachineTypeAndModel_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqMachineTypeAndModel
     *
     * Il client ha mandato una richiesta AJAX per conosce il machine type (espresso o instant) e il model (un numero che indentifica il 
     *  modello, tipo BL o Fusion)
     *
        Input:
            command: getMachineTypeAndModel
            params:  none

        Output
		    json
		    {
			    mType: numero 1 se espresso, 2 se instant
			    mModel: numero intero 8bit
			    isInduzione: numero intero 8bit, vale 0 o 1
                aliChina: numero intero 8bit, vale 0 o 1 che indica se il thread AlipayChina è stato lanciato
                motherboard: variscite|D23
		    }
     */


    class CmdHandler_ajaxReqMachineTypeAndModel : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqMachineTypeAndModel(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getMachineTypeAndModel"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqMachineTypeAndModel_h_
