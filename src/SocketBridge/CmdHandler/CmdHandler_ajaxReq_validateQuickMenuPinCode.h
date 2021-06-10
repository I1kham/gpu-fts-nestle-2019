#ifndef _CmdHandler_ajaxReq_validateQuickMenuPinCode_h_
#define _CmdHandler_ajaxReq_validateQuickMenuPinCode_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_validateQuickMenuPinCode
     *
     *  Il client vuole verificare che il (quick menu) pincode immesso sia valido
        Input:
            command: checkQuickMenuPinCode
            params:
				pin: stringa con il pin (numerico)

		Output
			OK
            KO
     */


    class CmdHandler_ajaxReq_validateQuickMenuPinCode : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_validateQuickMenuPinCode(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "checkQuickMenuPinCode"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_validateQuickMenuPinCode_h_
