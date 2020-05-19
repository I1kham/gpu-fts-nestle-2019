/******************************************************************************************************************
    CmdHandler_ajaxReq

    Classe di base per tutti i comandi di tipo "ajax".
    Per una descrizione sommaria di cosa sia un comando "ajax", vedi SocketBridge.h

    I comandi ajax sono identificati da un nome che deve essere univoco per comando e deve essere ritornato dal metodo statico:
        static const char*  getCommandName()
    da implementarsi in ogni classe derivata da CmdHandler_ajaxReq.

    Ogni nuova classe di comando ajax, deve essere aggiunta al file CmdHandler_ajaxReq.cpp nel metodo CmdHandler_ajaxReqFactory::spawn()
    usando la macro CHECK. Questo è necessario per implementare il meccanismo di instanziamento dinamico delle classi (ovvero, data la
    stringa con il nome del comando, istanza la classe esatta che gestisce quel comando).

    SocketBridge istanzia comandi di tipo "ajax" solo alla ricezione di un paritcolare msg via socket.
    Tale messaggio riporta il nome del comando oltre agli eventuali parametri.
    Alla ricezione di un tale msg, SocketBridge scorre l'elenco dei cmd ajax che conosce fino a che non ne trova uno il cui
    getCommandName() sia == al nome comando ricevuto dal messaggio.

    Alla ricezione di un (valido) comando ajax, SocketBridge ha due opzioni, a seconda di come è stato pensato il comando:
        1)  passare l'evento direttamente a CPUBridge
        2)  gestire l'evento internamente

    Nel caso 1), la richiesta (ad es: richiesta prezzi selezione) viene passata direttamente a CPUBridge.
    E' compito di CPUBridge gestire la richiesta ed eventualmente produrre una risposta.
    La risposta di CPUBridge viene passata indietro a SocketBridge (tramite una "notify") insieme con l'indicazione di "chi" ha iniziato la richiesta.
    Il "chi" è esattamente l'istanza del comando che SocketBridge ha creato alla ricezione del msg via socket.
    Il metodo onCPUBridgeNotification() di quella istanza viene chiamato in modo da poter fornire una risposta al client che aveva fatto la richiesta.
        
    Nel caso 2), la gestione dell'evento è tutta interna a SocketBridge, non è richiesta la collaborazione della CPU.
    Il procedimento è analogo al punto precedente, tranne per il fatto che non viene fatta alcuna richiesta a CPUBridge.
    Il metodo onCPUBridgeNotification() dell'istanza del comando viene chiamato in modo da poter fornire una risposta al client che ha fatto la richiesta.

*/
#ifndef _CmdHandler_ajaxReq_h_
#define _CmdHandler_ajaxReq_h_
#include "CmdHandler.h"


namespace socketbridge
{
    /*********************************************************
     * CmdHandler_ajaxReq
     *
     * Classe di base per la gestione delle richieste "ajax"
     */
    class CmdHandler_ajaxReq : public CmdHandler
    {
    public:
                                CmdHandler_ajaxReq (const HSokBridgeClient &identifiedClientHandle, u16 handlerID, u64 dieAfterHowManyMSec, u8 ajaxRequestID) :
                                    CmdHandler(identifiedClientHandle, handlerID, dieAfterHowManyMSec)
                                {
                                    this->ajaxRequestID = ajaxRequestID;
                                }

        virtual void            passDownRequestToCPUBridge (cpubridge::sSubscriber &from, const u8 *params) = 0;
		//virtual void			onCPUBridgeNotification (socketbridge::Server *server, HSokServerClient &hClient, const rhea::thread::sMsg &msgFromCPUBridge) = 0;
		//virtual bool			needToPassDownToCPUBridge() const = 0;

        virtual void			handleRequestFromSocketBridge (socketbridge::Server *server UNUSED_PARAM, HSokServerClient &hClient UNUSED_PARAM, const u8 *params UNUSED_PARAM)
								{
									//le classi derivate devono implementare questa fn SOLO se il messaggio in questione è di quelli che
									//viene gestito direttamente da socketBridge senza passare per CPUBridge (es: CmdHandler_eventReqClientList che chiede una lista dei client
									//connessi alla socket).
									//Se invece si tratta di un msg che deve essere passato al CPUBrige, i metodi da implementare sono passDownRequestToCPUBridge() e
									//onCPUBridgeNotification()
									DBGBREAK;
								}

    protected:

    protected:
        u8                      ajaxRequestID;
    };






    /*********************************************************
     * CmdHandler_ajaxReqFactory
     *
     *
     */
    class CmdHandler_ajaxReqFactory
    {
    public:
        static CmdHandler_ajaxReq* spawn (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec,
                                                   const u8 **out_params);
    };
} // namespace socketbridge

#endif // _CmdHandler_ajaxReq_h_
