#ifndef _IdentifiedClientList_h_
#define _IdentifiedClientList_h_
#include "SocketBridgeEnumAndDefine.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"

namespace socketbridge
{
    /***************************************************
     *	IdentifiedClientList
     *
     *  Mantiene una lista di client che si sono connessi tramite la socket.
	 *	Questa classe non è thread-safe
     */
    class IdentifiedClientList
    {
    public:
        struct sInfo
        {
			HSokBridgeClient	handle;					
			u32					currentWebSocketHandleAsU32;		//HSokServerClient  (invalid se la connessione al momento è chiusa)
			
			SokBridgeIDCode		idCode;								//codice univoco di identificazione
			SokBridgeClientVer	clientVer;
            
			u64     timeCreatedMSec;								//timestamp della creazione del record
            u64     lastTimeConnectedMSec;							//timestamp dell'ultima connessione (bind della socket)
			u64     lastTimeDisconnectedMSec;						//timestamp dell'ultima disconnessione (unbind della socket)
        };

    public:
							IdentifiedClientList()                          { }
							~IdentifiedClientList()                         { unsetup(); }

        void				setup (rhea::Allocator *allocator);
        void				unsetup();

		const sInfo*		isKwnownSocket (const HSokServerClient &h) const;
							/*	!=NULL se [h] è attualmente in lista.
								NULL altrimenti
							*/
		
		const sInfo*		isKnownIDCode (const SokBridgeIDCode &idCode) const;
							/*	true se [idCode] è attualmente in lista. In questo caso, ritorna un valido [out_handle].
								false altrimenti
							*/

		HSokBridgeClient&	newEntry(const SokBridgeIDCode &idCode, u64 timeNowMSec);
							/*	crea un nuovo record e gli associa [idCode].
								Il nuovo record non è bindato ad alcuna socket.
								RItorna il suo handle
							*/

		void				updateClientVerInfo (const HSokBridgeClient &handle, const SokBridgeClientVer &v);
		bool				compareClientVerInfo(const HSokBridgeClient &handle, const SokBridgeClientVer &v) const;
							/*	true se [v] == sInfo->clientVer del client puntato dal [handle]
							*/

		void				bindSocket (const HSokBridgeClient &handle, const HSokServerClient &hSok, u64 timeNowMSec);
		void				unbindSocket (const HSokBridgeClient &handle, u64 timeNowMSec);


    private:
		u32				priv_findByHSokServerClient (const HSokServerClient &h) const;
		u32				priv_findByHSokBridgeClient (const HSokBridgeClient &h) const;
		u32				priv_findByIDCode (const SokBridgeIDCode &idCode) const;

	private:
		rhea::FastArray<sInfo>  list;
		HSokBridgeClient		nextHandle;
	};
} // namespace socketbridge
#endif // _IdentifiedClientList_h_
