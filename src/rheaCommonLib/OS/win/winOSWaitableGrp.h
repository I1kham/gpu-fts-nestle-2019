#ifdef WIN32
#ifndef _winOSWaitableGrp_h_
#define _winOSWaitableGrp_h_
#include "winOS.h"
#include "winOSSocket.h"



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
	static const u8 MAX_EVENTS_HANDLE_PER_CALL = 16;

	enum eEventOrigin
	{
		evt_origin_socket = 1,
		evt_origin_osevent = 2,
		evt_origin_serialPort = 3,
		evt_origin_deleted = 4
	};
public:
					OSWaitableGrp();
					~OSWaitableGrp();

	bool            addSocket(OSSocket &sok, void *userParam = NULL)					{ sRecord *s = priv_addSocket(sok); if (s) s->userParam.asPtr = userParam; return (s != NULL); }
	bool            addSocket(OSSocket &sok, u32 userParam)								{ sRecord *s = priv_addSocket(sok); if (s) s->userParam.asU32 = userParam; return (s != NULL); }
	void            removeSocket(OSSocket &sok);

	bool            addEvent(const OSEvent &evt, void *userParam = NULL)				{ sRecord *s = priv_addEvent(evt); if (s) s->userParam.asPtr = userParam; return (s != NULL); }
	bool            addEvent(const OSEvent &evt, u32 userParam)							{ sRecord *s = priv_addEvent(evt); if (s) s->userParam.asU32 = userParam; return (s != NULL); }
	void            removeEvent(const OSEvent &event);

/*	bool            addSerialPort(const OSSerialPort &sp, void *userParam = NULL)		{ sRecord *s = priv_addSerialPort(sp); if (s) s->userParam.asPtr = userParam; return (s != NULL); }
	bool            addSerialPort(const OSSerialPort &sp, u32 userParam)				{ sRecord *s = priv_addSerialPort(sp); if (s) s->userParam.asU32 = userParam; return (s != NULL); }
	void            removeSerialPort(const OSSerialPort &sp)							{ priv_onRemove(sp.fd); }
*/

	u8              wait(u32 timeoutMSec);
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

	eEventOrigin    getEventOrigin(u8 iEvent) const;
					/* ritorna il tipo di oggetto che ha generato l'evento i-esimo
					 */

	void*           getEventUserParamAsPtr(u8 iEvent) const;
	u32             getEventUserParamAsU32(u8 iEvent) const;
					/* ritorna lo "userParam" così come definito durante la chiamana addSocket() e/o addEvent()
					 */

	OSSocket&       getEventSrcAsOSSocket(u8 iEvent) const;
					/* se getEventOrigin() == evt_origin_socket, ritorna la soket che ha scatenato l'evento
					 */

	OSEvent&        getEventSrcAsOSEvent(u8 iEvent) const;
					/* come sopra */

	//OSSerialPort&   getEventSrcAsOSSerialPort(u8 iEvent) const;
					/* come sopra */

private:
	static const u8	MAX_EVENTS_RETURNED = 32;

	struct sIfSocket
	{
		OSSocket	sok;
		HANDLE		hEventNotify;
	};

	struct sIfEvent
	{
		OSEvent		evt;
	};

	union sOrigin
	{
		sIfSocket	osSocket;
		sIfEvent	osEvent;
	};

	union uUserParam
	{
		void    *asPtr;
		u32     asU32;
	};

	struct sRecord
	{
		sOrigin     origin;
		u8          originType;
		uUserParam  userParam;
		sRecord     *next;
	};

private:
	sRecord*        priv_newRecord();
	void            priv_removeHandle(HANDLE h);
	sRecord*        priv_addSocket(OSSocket &sok);
	sRecord*        priv_addEvent(const OSEvent &evt);
	sRecord*        priv_addSerialPort(const OSSerialPort &sp);
	u8              priv_wait(u32 timeoutMSec);

private:
	sRecord         *base;
	HANDLE			eventsHandle[MAX_EVENTS_HANDLE_PER_CALL];
	u32				nEventsReady;
	sRecord*		generatedEventList[MAX_EVENTS_RETURNED];

	u8				debug_bWaiting;
};




#endif // _winOSWaitableGrp_h_
#endif //WIN32
