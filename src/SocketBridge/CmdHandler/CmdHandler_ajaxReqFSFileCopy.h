#ifndef _CmdHandler_ajaxReqFSFileCopy_
#define _CmdHandler_ajaxReqFSFileCopy_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFSFileCopy
     *
     * Il client ha mandato una richiesta AJAX per copiare un file da un folder ad un altro
     *
        Input:
            command: FSCopy
            pSRC:	absolutePathToSRCFolder
			fSRC:	filename src
			pDST:	absolutePathToDSTFolder
			fDST:	filename dst

        Output
			OK
			KO
     */
    class CmdHandler_ajaxReqFSFileCopy : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFSFileCopy(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSCopy"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFSFileCopy_
