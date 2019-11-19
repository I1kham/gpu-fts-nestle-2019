#ifndef _CmdHandler_eventReqGetPosizioneMacina_h_
#define _CmdHandler_eventReqGetPosizioneMacina_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqGetPosizioneMacina
     *
     *
     *
     */
    class CmdHandler_eventReqGetPosizioneMacina : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType_getPosizioneMacina;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTITFY_POSIZIONE_MACINA;

		CmdHandler_eventReqGetPosizioneMacina(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																								{ return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqGetPosizioneMacina_h_
