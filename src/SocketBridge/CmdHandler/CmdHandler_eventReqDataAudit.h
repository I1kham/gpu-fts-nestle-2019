#ifndef _CmdHandler_eventReqDataAudit_h_
#define _CmdHandler_eventReqDataAudit_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqDataAudit
     *
     *  Il client vuole scaricare il DataAudit.
     *  La richiesta viene passata a CPUBridge la quale risponde con una serie di notifiche CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS a tutti i client
     *  connessi.
     *  La notifica finale CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS che indica se il download è stato terminato con successo oppure no, viene
     *  inviata solo al client che ha iniziato la richiesta.
     *
     */
    class CmdHandler_eventReqDataAudit : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_reqDataAudit;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS;

		CmdHandler_eventReqDataAudit(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec UNUSED_PARAM) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, 120000) //override del tempo di morte perchè questo comando potrebbe impiegare un bel po' 
                    {  }																//di tempo prima di terminare, visto che chiede alla CPU il dataAudit


		bool		needToPassDownToCPUBridge() const																		{ return true; }
		void        passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
		void        onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

    };
} // namespace socketbridge

#endif // _CmdHandler_eventReqDataAudit_h_
