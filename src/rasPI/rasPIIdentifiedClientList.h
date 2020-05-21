#ifndef _rasPIIdentifiedClientList_h_
#define _rasPIIdentifiedClientList_h_
#include "../SocketBridge/SocketBridgeEnumAndDefine.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/Protocol/ProtocolSocketServer.h"
#include "../rheaCommonLib/rheaRandom.h"

namespace socketbridge
{
    /***************************************************
     *	IdentifiedClientList
     *
	 *	ATTENZIONE: questo è un copia incolla dei file IdentifiedClientList.cpp/IdentifiedClientList.h appartenenti al progetto
     *				socketBridge.
	 *				Fare riferimento ai file originali e cercare di mantenere questa copia uguale all'originale
     */
    class IdentifiedClientList
    {
    public:
							IdentifiedClientList()							{ }
							~IdentifiedClientList()                         { unsetup(); }

        void				setup (rhea::Allocator *allocator);
        void				unsetup();

		const sIdentifiedClientInfo*		isKwnownSocket (const HSokServerClient &h, u64 timeNowMSec) ;
							/*	!=NULL se [h] è attualmente in lista.
								NULL altrimenti
								Se != NULL, aggiorna il lastTimeRCV
							*/
		
		const sIdentifiedClientInfo*		isKnownIDCode (const SokBridgeIDCode &idCode);
							/*	true se [idCode] è attualmente in lista. In questo caso, ritorna un valido [out_handle].
								false altrimenti
							*/

		HSokBridgeClient&	newEntry(u64 timeNowMSec, SokBridgeIDCode *out_idCode);
							/*	crea un nuovo record e gli associa idCode univoco che ritorna in [out_idCode].
								Il nuovo record non è bindato ad alcuna socket.
								RItorna il suo handle
							*/

		void				updateClientVerInfo (const HSokBridgeClient &handle, const SokBridgeClientVer &v);
		bool				compareClientVerInfo(const HSokBridgeClient &handle, const SokBridgeClientVer &v) const;
							/*	true se [v] == sInfo->clientVer del client puntato dal [handle]
							*/

		bool				bindSocket (const HSokBridgeClient &handle, const HSokServerClient &hSok, u64 timeNowMSec);
		void				unbindSocket (const HSokBridgeClient &handle, u64 timeNowMSec);

		void				purge(u64 timeNowMSec);
							//elimina i record che hanno attualmente la socket non bindata ed il cui ultimo rcv risale a più di 60 sec

		//void				markAsHavPendingRequest(const HSokBridgeClient &handle, bool b);
		//bool				hasPendingRequest(const HSokBridgeClient &handle) const;

		u32					getCount() const																					{ return list.getNElem();  }
		bool				getHandleByIndex(u32 i, HSokBridgeClient *out_handle) const;
		const sIdentifiedClientInfo*		getInfoByIndex(u32 i) const;
		bool				isBoundToSocket (const HSokBridgeClient &handle, HSokServerClient *out_h) const;

    private:
		u32					priv_findByHSokServerClient (const HSokServerClient &h) const;
		u32					priv_findByHSokBridgeClient (const HSokBridgeClient &h) const;
		u32					priv_findByIDCode (const SokBridgeIDCode &idCode) const;
		HSokBridgeClient&	priv_createHandle (u64 timeNowMSec, const SokBridgeIDCode &idCode);

	private:
		rhea::FastArray<sIdentifiedClientInfo>  list;
		HSokBridgeClient		nextHandle;
	};
} // namespace socketbridge
#endif // _rasPIIdentifiedClientList_h_
