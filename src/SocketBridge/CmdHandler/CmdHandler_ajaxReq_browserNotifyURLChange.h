#ifndef _CmdHandler_ajaxReq_browserNotifyURLChange_h_
#define _CmdHandler_ajaxReq_browserNotifyURLChange_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_browserNotifyURLChange
     *
     *  Il browser della GUI notifica che ha intenzione di passare ad una nuova URL.
     *  Questo comando serve per gestire la transizione dalla versione con browser direttamente in QT (nel quale era possibile intercettare i cambi di URL)
     *  alla versione browser standalone da utilizzarsi nelle rhTT1
        Input:
            command: browserURLChange
            params:
				url: stringa
		Output
			OK
     */


    class CmdHandler_ajaxReq_browserNotifyURLChange : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_browserNotifyURLChange(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "browserURLChange"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_browserNotifyURLChange_h_
