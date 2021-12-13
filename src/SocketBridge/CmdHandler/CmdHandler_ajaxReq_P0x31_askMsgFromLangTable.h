#ifndef _CmdHandler_ajaxReq_P0x31_askMsgFromLangTable_h_
#define _CmdHandler_ajaxReq_P0x31_askMsgFromLangTable_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x31_askMsgFromLangTable
     *
     * la GUI ha mandato una richiesta AJAX per conoscere il messaggio n-esimo della tabella linguaggi i-esima di CPU
     *
        Input:
            command: askMsgFromLangTable
            params:
				tabID: 0..255
				rowNum: 0..255
                lang: 1 | 2

		Output
            JSON
			{
				"tabID":
				"rowNum": 
				"msg" : "mesaggio in UTF8"
			}
     */


    class CmdHandler_ajaxReq_P0x31_askMsgFromLangTable : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x31_askMsgFromLangTable(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "askMsgFromLangTable"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x31_askMsgFromLangTable_h_
