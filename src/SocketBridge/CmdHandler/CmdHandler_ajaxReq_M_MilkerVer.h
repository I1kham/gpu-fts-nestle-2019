#ifndef _CmdHandler_ajaxReq_M_MilkerVer_h_
#define _CmdHandler_ajaxReq_M_MilkerVer_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_M_MilkerVer
     *
     *	Il client ha mandato una richiesta AJAX per conosce la versione (in formato stringa)
	 *	del cappuccinatore
     *
        Input:
            command: milkver
            params:  none

        Output
			stringa in utf8
     */


    class CmdHandler_ajaxReq_M_MilkerVer : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReq_M_MilkerVer(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "milkver"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_M_MilkerVer_h_
