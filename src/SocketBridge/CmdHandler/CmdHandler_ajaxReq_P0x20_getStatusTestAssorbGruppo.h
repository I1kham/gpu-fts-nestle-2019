#ifndef _CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo_h_
#define _CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo_h_
#include "../CmdHandler_ajaxReq.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo
     *
     * la GUI ha mandato una richiesta AJAX per conosce lo stato di avanzamento del test "asorbimento gruppo"
     *
        Input:
            command: getStatTestAssGrp
            params:  none

        Output
        json
        {
            fase: numero intero 8bit che indica la fase attuale del test
						[fase] == 0 => test non iniziato
						[fase] == 4 => test avviato
						[fase] == 1:3 => ciclo di test 1:3
						[fase] == 5 => test terminato

            esito: numero intero 8bit
						[esito] == 0  => OK
						[esito] == 1  => KO
            r1up: stringa che indica "Assorbimento medio in salita"
			r1down: stringa che indica "Assorbimento medio in discesa"
			
			r2up: stringa che indica "Assorbimento massimo in salita"
			r2down: stringa che indica "Assorbimento massimo in discesa"
		            
			r3up: stringa che indica "Tempo in salita"
			r3down: stringa che indica "Tempo in discesa"

			r4up: stringa che indica "Assorbimento medio in salita durante il ciclo di test 1"
			r4down: stringa che indica "Assorbimento medio in discesa durante il ciclo di test 1"

			r5up: stringa che indica "Assorbimento medio in salita durante il ciclo di test 2"
			r5down: stringa che indica "Assorbimento medio in discesa durante il ciclo di test 2"

			r6up: stringa che indica "Assorbimento medio in salita durante il ciclo di test 3"
			r6down: stringa che indica "Assorbimento medio in discesa durante il ciclo di test 3"
}
     */


    class CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo : public CmdHandler_ajaxReq
    {
    public:
		CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo(const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                CmdHandler_ajaxReq(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID)
                                { }

		bool				needToPassDownToCPUBridge() const { return true; }
        void                passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params);
        void                onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge);

        static const char*  getCommandName()                            { return "getStatTestAssGrp"; }
    };

} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo_h_
