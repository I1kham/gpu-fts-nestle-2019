#ifndef _CmdHandler_ajaxReqMilkerType_h_
#define _CmdHandler_ajaxReqMilkerType_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqMilkerType
     *
     * Il client ha mandato una richiesta AJAX per conosce il milker type, ovvero il tipo di cappuccinatore attualmente 
     *  connesso alla macchina
     *
        Input:
            command: getMilkerType
            params:  none

        Output
		    json
		    {
			    mType: intero 8 bit => 0=none, 1=venturi, 2=capp induzione
		    }
     */


    class CmdHandler_ajaxReqMilkerType : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqMilkerType(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const												{ return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getMilkerType"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqMilkerType_h_
