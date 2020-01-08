#ifndef _CmdHandler_ajaxReqGetOFFList_h_
#define _CmdHandler_ajaxReqGetOFFList_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReqGetOFFList
     *
     *	la GUI ha mandato una richiesta AJAX per richiedere la lista degli OFF di CPU
	 *	La lista completa degli off potrebbe necessitare di piu' di un messaggio. Se nella risposta, nextIndex != 0, allora è necessario
	 *	chiamare nuovamente il comando usando startIndex=nextIndex
     *
        Input:
            command: getOFFList
            startIndex:  da 0 a N

        Output
            JSON
			{
				"nextIndex":13,
				"yyyy":[2020,2019],
				"mm":["01","02"],
				"dd":["03","04"],
				"hh":["03","04"],
				"min":["03","04"],
				"codice":["8","7"],
				"tipo":["A"," "],
				"stato":["0","1"]
			}

			Il parametro [isLastBlock] viene messo a 1 quando non ci sono ulteriori OFF da recuperare.
			Ogni risposta prevede un numero variabile di OFF. Per conoscere quanti OFF sono stati riportati dalla risposta, usare anno.length
     */
    class CmdHandler_ajaxReqGetOFFList : public CmdHandler_ajaxReq
    {
    public:
							CmdHandler_ajaxReqGetOFFList(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }
							
		bool				needToPassDownToCPUBridge() const { return true; }
		void                passDownRequestToCPUBridge(cpubridge::sSubscriber &from, const char *params);
		void                onCPUBridgeNotification(socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getOFFList"; }

    };
} // namespace socketbridge
#endif // _CmdHandler_ajaxReqGetOFFList_h_
