#ifndef _CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition_h_
#define _CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition
     *
     *  GPU vuole conoscere l'attuale ripetizione N-esemina di M della selezione (con JUG) in corso.
     *  Se una selezione con JUG è in corso, CPU riporta l'attuale [N]-esima ripetizione e il numero totale [M] di ripetizioni.
     *  Se una selezione senza JUG è in corso (oppure nessuna selezione), CPU riporta [N]=0 e [M]=0
        Input:
            command: getJugCurrentRepetition
            params: none

		Output
			{numberOf: <number>, total: <number>}
            KO
     */


    class CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getJugCurrentRepetition"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition_h_
