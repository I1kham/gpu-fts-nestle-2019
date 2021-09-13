#ifndef _CmdHandler_ajaxReq_P0x25_caffeCortesia_h_
#define _CmdHandler_ajaxReq_P0x25_caffeCortesia_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x25_caffeCortesia
     *
     *	Chiude a CPU di effettuare il caffè di cortesia, ovvero quella bevanda che viene prodotto alla fine del
	 *	lavaggio sanitario la cui ricetta non è modificabile dall'utente ed è hardcoded nella CPU
     *
        Input:
            command: runCaffeCortesia
            params:
				m: intero da 1 a 4 ad indicare la macina da usare

        Output:
			"OK"

		Alla ricezione di questo comando, CPU passa in stato PREP_BEVANDA per tutta la durata del caffè
     */
    class CmdHandler_ajaxReq_P0x25_caffeCortesia : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x25_caffeCortesia(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "runCaffeCortesia"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x25_caffeCortesia_h_
