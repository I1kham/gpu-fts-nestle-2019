#ifndef _CmdHandler_ajaxReqTaskSpawn_
#define _CmdHandler_ajaxReqTaskSpawn_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqTaskSpawn
     *
     * la GUI ha mandato una richiesta AJAX per spawnare un task
     *
        Input:
            command: taskSpawn
            name:	 nome del task da spawnare
			params:	 stringa con l'elenco dei parametri da passare al task

        Output:
			0	=> errore
			>0	=> id del task da utilizzare col comando taskStatus
     */
    class CmdHandler_ajaxReqTaskSpawn : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqTaskSpawn(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const			{ return false; }
		void				handleRequestFromSocketBridge(socketbridge::Server *server, HSokServerClient &hClient, const char *params);

        void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const char *params UNUSED_PARAM) {}
        void                onCPUBridgeNotification(socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM) {}

        static const char*  getCommandName()                            { return "taskSpawn"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqTaskSpawn_
