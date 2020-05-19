#ifndef _CmdHandler_ajaxReqFSFileList_
#define _CmdHandler_ajaxReqFSFileList_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFSFileList
     *
     * Il client ha mandato una richiesta AJAX per conosce l'elenco dei file di un folder
     *
        Input:
            command: FSList
            path:	absolutePathToFolder
			jolly:	filtro sui file

        Output
        json
        {
			path:		absolutePathToFolder
			up:			0 | 1		1 se è possibile effettuare un ".." sul path attuale
            folderList:	stringa con i folder name separati da §
            fileList: stringa con i file name separati da §
        }
     */
    class CmdHandler_ajaxReqFSFileList : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFSFileList(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSList"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFSFileList_
