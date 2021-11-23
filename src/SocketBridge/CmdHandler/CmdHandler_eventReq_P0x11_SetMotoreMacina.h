#ifndef _CmdHandler_eventReq_P0x11_SetMotoreMacina_h_
#define _CmdHandler_eventReq_P0x11_SetMotoreMacina_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReq_P0x11_SetMotoreMacina
     *
     * params:
		u8 macina 1 o 2
		u8 tipo di movimento (apri chiudi stop), vedi cpubridge::eCPUProg_macinaMove
     *
     */
    class CmdHandler_eventReq_P0x11_SetMotoreMacina : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::setMotoreMacina;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MOTORE_MACINA;

		CmdHandler_eventReq_P0x11_SetMotoreMacina(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																								{ return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReq_P0x11_SetMotoreMacina_h_
