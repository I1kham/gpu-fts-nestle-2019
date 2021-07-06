#ifndef _CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc_h_
#define _CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc
     *
     *
        Input:
            command: notifyEndGrinClean
            params:
				m: 1 o 2 (macina 1 o macina 2)

		Output
			OK

		GPU notifica CPU che la procedura di grinder cleaning sulla macina [m] è stata portata a termine con successo
     */


    class CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "notifyEndGrinClean"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqAttivazioneMotore_h_
