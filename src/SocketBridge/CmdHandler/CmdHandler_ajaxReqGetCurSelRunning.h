#ifndef _CmdHandler_ajaxReqGetCurSelRunning_h_
#define _CmdHandler_ajaxReqGetCurSelRunning_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetCurSelRunning
     *
     * Il client ha mandato una richiesta AJAX per conosce il numero della selezione attualmente in esecuzione
     *
        Input:
            command: getCurSelRunning
            params:  none

        Output
            stringa rappresentante il numero della selezione in esecuzione ("0" se non ci sono selezioni in corso)
     */


    class CmdHandler_ajaxReqGetCurSelRunning : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_ajaxReqGetCurSelRunning (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getCurSelRunning"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetCurSelRunning_h_
