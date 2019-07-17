#ifdef LINUX
#ifndef _linuxOSWaitableGrp_h_
#define _linuxOSWaitableGrp_h_
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "linuxOS.h"
#include "linuxOSSocket.h"



/********************************************************************
 * OSWaitableGrp
 *
 * E' un oggetto che accetta altri oggetti (di tipo socket e/o event) e poi espone una funzione
 * wait() che è in grado di sospendere l'esecuzione fino a che uno (o più) qualunque degli oggetti che gli
 * sono stati "addati" non genera un evento.
 *
 * Nel caso degli OSEvent, è sufficiente chiamare il relativo metodo fire() per far scattare l'evento.
 * Nel caso di OSSocket, l'evento scatta quando ci sono dei dati pronti per essere read(), o quando la socket viene disconnessa.
 *
 */
class OSWaitableGrp
{
public:
    static const u8 MAX_EVENTS_PER_CALL = 8;

    enum eEventOrigin
    {
        evt_origin_socket = 1,
        evt_origin_osevent = 2,
        evt_origin_serialPort = 3
    };
public:
                    OSWaitableGrp();
                    ~OSWaitableGrp();

    bool            addSocket (const OSSocket &sok, void *userParam=NULL)                       { sRecord *s=priv_addSocket(sok); if(s) s->userParam.asPtr=userParam; return (s != NULL); }
    bool            addSocket (const OSSocket &sok, u32 userParam)                              { sRecord *s=priv_addSocket(sok); if(s) s->userParam.asU32=userParam; return (s != NULL); }
    void            removeSocket (const OSSocket &sok)                                          { priv_onRemove (sok.socketID); }

    bool            addEvent (const OSEvent &evt, void *userParam=NULL)                         { sRecord *s=priv_addEvent(evt); if(s) s->userParam.asPtr=userParam; return (s != NULL); }
    bool            addEvent (const OSEvent &evt, u32 userParam)                                { sRecord *s=priv_addEvent(evt); if(s) s->userParam.asU32=userParam; return (s != NULL); }
    void            removeEvent (const OSEvent &event)                                          { priv_onRemove (event.evfd); }

    bool            addSerialPort (const OSSerialPort &sp, void *userParam=NULL)                { sRecord *s=priv_addSerialPort(sp); if(s) s->userParam.asPtr=userParam; return (s != NULL); }
    bool            addSerialPort (const OSSerialPort &sp, u32 userParam)                       { sRecord *s=priv_addSerialPort(sp); if(s) s->userParam.asU32=userParam; return (s != NULL); }
    void            removeSerialPort (const OSSerialPort &sp)                                   { priv_onRemove (sp.fd); }


    u8              wait (u32 timeoutMSec);
                    /* Per specificare un tempo di wait "infinito", usare timeoutMSec=u32MAX
                     * Per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
                     * Tutti gli altri valori sono comunque validi ma non assumono significati particolari
                     *
                     * La chiamata è bloccante per almeno [timeoutMSec]
                     * Ritorna il numero di eventi ricevuti oppure 0 se non sono stati ricevuti eventi ed il timeout è scaduto
                     * Nel caso di eventi ricevuti, usare getEventOrigin() per conoscere il tipo di oggetto che ha generato
                     * l'evento i-esimo (es: OSSocket oppure OSEvent) e usare la getEventSrc() per ottenere il puntatore all'oggetto
                     *
                     * Il numero massimo di eventi per chiamata è MAX_EVENTS_PER_CALL
                     * Ad ogni chiamata di wait(), eventuali eventi precedentemente ritornati andranno persi
                     */

    eEventOrigin    getEventOrigin (u8 iEvent) const;
                    /* ritorna il tipo di oggetto che ha generato l'evento i-esimo
                     */

    void*           getEventUserParamAsPtr (u8 iEvent) const;
    u32             getEventUserParamAsU32 (u8 iEvent) const;
                    /* ritorna lo "userParam" così come definito durante la chiamana addSocket() e/o addEvent()
                     */

    OSSocket&       getEventSrcAsOSSocket (u8 iEvent) const;
                    /* se getEventOrigin() == evt_origin_socket, ritorna la soket che ha scatenato l'evento
                     */

    OSEvent&        getEventSrcAsOSEvent (u8 iEvent) const;
                    /* come sopra */

    OSSerialPort&   getEventSrcAsOSSerialPort (u8 iEvent) const;
                    /* come sopra */

private:
    static const u8 WHATIS_SOCKET = 1;
    static const u8 WHATIS_EVENT = 2;

    union sOrigin
    {
        OSSocket osSocket;
        OSEvent  osEvent;
        OSSerialPort osSerialPort;
    };

    union uUserParam
    {
        void    *asPtr;
        u32     asU32;
    };

    struct sRecord
    {
        epoll_event eventInfo;
        sOrigin     origin;
        u8          originType;
        uUserParam  userParam;
        sRecord     *next;
    };

private:
    sRecord*        priv_newRecord (u32 flags);
    void            priv_findAndRemoveRecordByFD (int fd);
    void            priv_onRemove (int fd);
    int             priv_getFd (const sRecord *s) const;
    sRecord*        priv_addSocket (const OSSocket &sok);
    sRecord*        priv_addEvent (const OSEvent &evt);
    sRecord*        priv_addSerialPort (const OSSerialPort &sp);

private:
    sRecord         *base;
    int             hfd;
    epoll_event     events[MAX_EVENTS_PER_CALL];
    u8              nEventsReady;


};

#endif // _linuxOSWaitableGrp_h_

#endif
