#ifndef _CmdHandler_ajaxReqTestSelection_h_
#define _CmdHandler_ajaxReqTestSelection_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqTestSelection
     *
     *
        Input:
            command: testSelection
            params:
				s: sel num
				d: deviceID (vedi cpubridge::eCPUProgrammingCommand_testSelectionDevice)

		Output
			OK
			KO
     */


    class CmdHandler_ajaxReqTestSelection : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqTestSelection(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "testSelection"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqTestSelection_h_
