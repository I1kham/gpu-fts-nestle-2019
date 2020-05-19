#ifndef _CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore_h_
#define _CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore
     *
     * la GUI ha mandato una richiesta AJAX per conosce lo stato di avanzamento del test "asorbimento gruppo"
     *
        Input:
            command: getStatTestAssMotorid
            params:  none

        Output
        json
        {
            fase: numero intero 8bit che indica la fase attuale del test
					[fase] == 0 => test non iniziato
					[fase] == 4 => test avviato
					[fase] == 1 => ciclo di salita
					[fase] == 2 => ciclo di discesa
					[fase] == 5 => test terminato

            esito: numero intero 8bit
						[esito] == 0  => OK
						[esito] == 1  => KO

            r1up: stringa che indica "Assorbimento medio in salita"
			r1down: stringa che indica "Assorbimento medio in discesa"
     */
    class CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getStatTestAssMotorid"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore_h_
