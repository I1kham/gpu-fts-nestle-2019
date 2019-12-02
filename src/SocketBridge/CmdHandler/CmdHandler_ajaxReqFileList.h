#ifndef _CmdHandler_ajaxReqFileList_
#define _CmdHandler_ajaxReqFileList_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFileList
     *
     * la GUI ha mandato una richiesta AJAX per conosce l'elenco dei file di un folder
     *
        Input:
            command: FSList
            path:	absolutePathToFolder
			jolly:	filtro sui file

        Output
        json
        {
			path:		absolutePathToFolder
			folderList:	array con i folder
			fileList: array con i file
        }
     */
    class CmdHandler_ajaxReqFileList : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFileList(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSList"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFileList_
