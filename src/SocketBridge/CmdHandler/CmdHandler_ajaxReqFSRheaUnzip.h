#ifndef _CmdHandler_ajaxReqFSRheaUnzip_
#define _CmdHandler_ajaxReqFSRheaUnzip_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFSRheaUnzip
     *
     * Il client ha mandato una richiesta AJAX per unzippare un file in un folder su HD
     *
        Input:
            command: FSUnzip
			src:	absolutePathToZipFile
            dst:	absolutePathToFolderWhereToExtract (folder must exists)
			mkdir:	0 | 1	=> se !=0, la directory [dst] viene creata, altrimenti deve esistere già

        Output
			OK
			KO
	*/
    class CmdHandler_ajaxReqFSRheaUnzip : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFSRheaUnzip(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const u8 *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSUnzip"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFSRheaUnzip_
