#ifndef _CmdHandler_eventReqActivateBuzzer_h_
#define _CmdHandler_eventReqActivateBuzzer_h_
#include "../CmdHandler_eventReq.h"



namespace socketbridge
{
    /*********************************************************
     * CmdHandler_eventReqActivateBuzzer
     *
     *  il client vuole attivare il buzzer di CPU.
     *  Il buzzer è pilotabile usando i seguentu 3 parametri:
     *      [numRepeat]
     *      [beepLen_dSec]
     *      [pausaTraUnBeepELAltro_dSec] 
     *
     * Per tutti e 3 i parametri, valori validi sono compresi tra 0 e 15 inclusi
     */
    class CmdHandler_eventReqActivateBuzzer : public CmdHandler_eventReq
    {
    public:
        static const eEventType EVENT_TYPE_FROM_SOCKETCLIENT = eEventType::activateBuzzer;
		static const u16		EVENT_ID_FROM_CPUBRIDGE = CPUBRIDGE_NOTIFY_MAX_ALLOWED;		//vuol dire che questa classe non risponde ad alcuna notifica di CPUBridge

		CmdHandler_eventReqActivateBuzzer(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec) :
                        CmdHandler_eventReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                    {  }

		bool		needToPassDownToCPUBridge() const																								{ return true; }
        void        passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *payload, u16 payloadLen);
        void        onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);
    };
} // namespace socketbridge
#endif // _CmdHandler_eventReqActivateBuzzer_h_
