#ifndef _CmdHandler_ajaxReqGetVoltageAndTemp_h_
#define _CmdHandler_ajaxReqGetVoltageAndTemp_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetVoltageAndTemp
     *
     * la GUI richiede il valore "live" del voltaggio e delle varie temperature (camera caffe', bollitore...)
     *
        Input:
            command: getVandT
            params:  none

        Output:
			json
			{
				tcam: temp camera 
				tbol: temp bollitore
				tcap: temp cappuccinatore
				v: voltaggio
			}
     */


    class CmdHandler_ajaxReqGetVoltageAndTemp : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReqGetVoltageAndTemp(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const char *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getVandT"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReqGetVoltageAndTemp_h_
