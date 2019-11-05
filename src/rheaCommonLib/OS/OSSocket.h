#ifndef _OSSocket_h_
#define _OSSocket_h_
#include "OSInclude.h"




inline void					OSSocket_init (OSSocket *sok)															{ platform::socket_init (sok); }
inline eSocketError         OSSocket_openAsTCPServer (OSSocket *out_sok, int portNumber)                            { return platform::socket_openAsTCPServer(out_sok, portNumber); }
inline eSocketError         OSSocket_openAsTCPClient (OSSocket *out_sok, const char *connectToIP, u32 portNumber)   { return platform::socket_openAsTCPClient(out_sok, connectToIP, portNumber); }

inline void                 OSSocket_close (OSSocket &sok)                                                          { platform::socket_close(sok); }

inline bool                 OSSocket_isOpen (const OSSocket &sok)                                                   { return platform::socket_isOpen(sok); }
                            /* false se la socket non è open.
                             * False anche a seguito di una chiamata a close() (in quanto la sok viene chiusa)
                             */

inline bool                 OSSocket_compare (const OSSocket &a, const OSSocket &b)                                 { return platform::socket_compare(a,b); }
                            /* true se "puntano" alla stessa socket
                            */


inline bool                 OSSocket_setReadTimeoutMSec  (OSSocket &sok, u32 timeoutMSec)                           { return platform::socket_setReadTimeoutMSec(sok, timeoutMSec); }
                            /* Per specificare un tempo di wait "infinito" (ie: socket sempre bloccante), usare timeoutMSec=u32MAX
                             * Per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
                             * Tutti gli altri valori sono comunque validi ma non assumono significati particolari
                             */

inline bool                 OSSocket_setWriteTimeoutMSec (OSSocket &sok, u32 timeoutMSec)                           { return platform::socket_setWriteTimeoutMSec(sok, timeoutMSec); }
                            /* Per specificare un tempo di wait "infinito" (ie: socket sempre bloccante), usare timeoutMSec=u32MAX
                             * Per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
                             * Tutti gli altri valori sono comunque validi ma non assumono significati particolari
                             */


inline bool					OSSocket_listen (const OSSocket &sok, u16 maxIncomingConnectionQueueLength = u16MAX)		{ return platform::socket_listen(sok, maxIncomingConnectionQueueLength); }
inline bool					OSSocket_accept (const OSSocket &sok, OSSocket *out_clientSocket)							{ return platform::socket_accept(sok, out_clientSocket); }

inline i32					OSSocket_read (OSSocket &sok, void *buffer, u16 bufferSizeInBytes, u32 timeoutMSec)			{ return platform::socket_read(sok, buffer, bufferSizeInBytes, timeoutMSec); }
                            /* prova a leggere dalla socket. La chiamata è bloccante per un massimo di timeoutMSec.
                             * Riguardo [timeoutMSec], valgono le stesse considerazioni indicate in setReadTimeoutMSec()
                             *
                             * Ritorna:
                             *      0   se la socket si è disconnessa
                             *      -1  se la chiamata avrebbe bloccato il processo (quindi devi ripetere la chiamata fra un po')
                             *      >0  se ha letto qualcosa e ha quindi fillato [buffer] con il num di bytes ritornato
                             */

inline i32                  OSSocket_write(OSSocket &sok, const void *buffer, u16 nBytesToSend)						{ return platform::socket_write(sok, buffer, nBytesToSend); }
							/*	Ritorna il numero di btye scritti sulla socket.
							 *	Se ritorna 0, vuol dire che la chiamata sarebbe stata bloccante e quindi
							 *	l'ha evitata
							 */


inline eSocketError			OSSocket_openAsUDP(OSSocket *out_sok)																{ return platform::socket_openAsUDP(out_sok); }
inline eSocketError			OSSocket_UDPbind(OSSocket &sok, int portNumber)														{ return platform::socket_UDPbind(sok, portNumber); }
inline void					OSSocket_UDPSendBroadcast(OSSocket &sok, const u8 *buffer, u32 nBytesToSend, int portNumber)		{ platform::socket_UDPSendBroadcast(sok, buffer, nBytesToSend, portNumber); }
inline u32					OSSocket_UDPSendTo(OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const OSNetAddr &addrTo)		{ return platform::socket_UDPSendTo(sok, buffer, nBytesToSend, addrTo); }
inline u32					OSSocket_UDPReceiveFrom(OSSocket &sok, u8 *buffer, u32 nMaxBytesToRead, OSNetAddr *out_from)		{ return platform::socket_UDPReceiveFrom(sok, buffer, nMaxBytesToRead, out_from); }
#endif //_OSSocket_h_
