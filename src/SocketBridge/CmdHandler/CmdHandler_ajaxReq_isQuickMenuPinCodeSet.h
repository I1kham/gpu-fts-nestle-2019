#ifndef _CmdHandler_ajaxReq_isQuickMenuPinCodeSet_h_
#define _CmdHandler_ajaxReq_isQuickMenuPinCodeSet_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_isQuickMenuPinCodeSet
     *
     *  Il client vuole sapere se il pin del quick menu è stato impostato
        Input:
            command: isQuickMenuPinCodeSet
            params: none

		Output
			Y
            N
     */


    class CmdHandler_ajaxReq_isQuickMenuPinCodeSet : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_isQuickMenuPinCodeSet(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "isQuickMenuPinCodeSet"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_isQuickMenuPinCodeSet_h_
