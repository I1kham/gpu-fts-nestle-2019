#ifndef _CmdHandler_eventReqClientList_h_
#define _CmdHandler_eventReqClientList_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqClientList
     *
     *  Il client vuole conosce l'elenco dei client connessi da SocketBrdige.
     *  La richiesta viene gestita interamente da SocketBridge senza l'ausilio di CPUBridge.
     *  Il metodo handleRequestFromSocketBridge() è quello che fa tutto quando, gli altri metodi sono vuoti
     *  a parte per needToPassDownToCPUBridge() che ritorna false
     *
     */
    class CmdHandler_eventReqClientList : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::reqClientList;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

		CmdHandler_eventReqClientList(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

        bool		needToPassDownToCPUBridge() const                                                                                                       { return false; }
        void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from UNUSED_PARAM, const u8 *payload UNUSED_PARAM, u16 payloadLen UNUSED_PARAM)		{ }
        void        onCPUBridgeNotification (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const rhea::thread::sMsg &msgFromCPUBridge UNUSED_PARAM)	{ }

		void		handleRequestFromSocketBridge (socketbridge::Server *server, HSokServerClient &hClient);
    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqClientList_h_
