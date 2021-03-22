#ifndef _CmdHandler_ajaxReqJugRepetitions_h_
#define _CmdHandler_ajaxReqJugRepetitions_h_
#include "../CmdHandler_ajaxReq.h"

namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqJugRepetitions
     *
     * Il client ha mandato una richiesta AJAX per conosce le ripetizioni in caso di JUG per ogni selezione disponibile
     *
        Input:
            command: jugRepetitions
            params:  none

        Output
        json
        {
            n: numero selezioni
            s: stringa coi le ripetizioni già formattate correttamente, ogni singolo carattere rappresenta la qauntita di ripetizioin per la specifica selezione
        }
     */
    
    class CmdHandler_ajaxReqJugRepetitions : public CmdHandler_ajaxReq
    {
    public:
                            CmdHandler_ajaxReqJugRepetitions (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "jugRepetitions"; }
    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqJugRepetitions_h_
