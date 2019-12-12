#ifndef _CmdHandler_ajaxReqFSDriveList_
#define _CmdHandler_ajaxReqFSDriveList_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqFSDriveList
     *
     * la GUI ha mandato una richiesta AJAX per conosce l'elenco dei drive di sistema
     *
        Input:
            command: FSDrvList

        Output
        json
        {
			drivePath:	array di drivePath   (es: "c:","d:")
			driveLabel: array di driveLabel (es: "OS","Backup")
			desktop:	path alla cartella desktop, se applicabile, oppure ""
        }
     */
    class CmdHandler_ajaxReqFSDriveList : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqFSDriveList(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "FSDrvList"; }


	private:
		char*				reallocString(rhea::Allocator *allocator, char *cur, u32 curSize, u32 newSize) const;
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqFSDriveList_
