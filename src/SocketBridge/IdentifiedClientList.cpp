#include "IdentifiedClientList.h"
#include "../rheaCommonLib/rheaDateTime.h"

using namespace socketbridge;


//*********************************************************
void IdentifiedClientList::setup (rhea::Allocator *allocator)
{
	nextHandle.index = 0;
    list.setup (allocator, 32);


}


//*********************************************************
void IdentifiedClientList::unsetup()
{
    list.unsetup();
}

//*********************************************************
u32 IdentifiedClientList::priv_findByHSokServerClient(const HSokServerClient &h) const
{
	const u32 hAsU32 = h.asU32();
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (list(i).currentWebSocketHandleAsU32 == hAsU32)
			return i;
	}
	return u32MAX;
}

//*********************************************************
u32 IdentifiedClientList::priv_findByHSokBridgeClient(const HSokBridgeClient &h) const
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (list(i).handle.index == h.index)
			return i;
	}
	return u32MAX;
}

//*********************************************************
bool IdentifiedClientList::getHandleByIndex (u32 i, HSokBridgeClient *out_handle) const
{
    if (i >= list.getNElem())
		return false;
	*out_handle = list(i).handle;
	return true;
}

//*********************************************************
const sIdentifiedClientInfo* IdentifiedClientList::getInfoByIndex(u32 i) const
{
    if (i >= list.getNElem())
		return NULL;
	return &list(i);
}

//*********************************************************
bool IdentifiedClientList::isBoundToSocket (const HSokBridgeClient &handle, HSokServerClient *out_h) const
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
		return false;

	if (u32MAX == list(i).currentWebSocketHandleAsU32)
		return false;

	out_h->initFromU32(list(i).currentWebSocketHandleAsU32);
	return true;
}


//*********************************************************
const sIdentifiedClientInfo* IdentifiedClientList::isKwnownSocket (const HSokServerClient &h, u64 timeNowMSec)
{
	u32 i = priv_findByHSokServerClient(h);
	if (u32MAX == i)
		return NULL;
	list[i].lastTimeRcvMSec = timeNowMSec;
	return &list(i);
}

//*********************************************************
u32 IdentifiedClientList::priv_findByIDCode (const SokBridgeIDCode &idCode) const
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (list(i).idCode == idCode)
			return i;
	}
	return u32MAX;
}


//*********************************************************
const sIdentifiedClientInfo* IdentifiedClientList::isKnownIDCode (const SokBridgeIDCode &idCode)
{
	//questo è per compatibilità con le GUI Fusion Beta v1, è da rimuovere prima o poi
	if (idCode.data.buffer[0] == 0x05 && idCode.data.buffer[1] == 0 && idCode.data.buffer[2] == 0 && idCode.data.buffer[3] == 0)
	{
		u32 i = priv_findByIDCode(idCode);
		if (u32MAX != i)
			return &list(i);

		//aggiungo un record d'ufficio per gestire questo caso
		priv_createHandle (rhea::getTimeNowMSec(), idCode);
		i = priv_findByIDCode(idCode);
		list[i].clientVer.apiVersion = 0x01;
		list[i].clientVer.appType = 0x02;
		list[i].clientVer.unused2 = 0x03;
		list[i].clientVer.unused3 = 0x04;
		return &list(i);
	}



	u32 i = priv_findByIDCode(idCode);
	if (u32MAX == i)
		return NULL;
	return &list(i);
}

//*********************************************************
HSokBridgeClient& IdentifiedClientList::newEntry(u64 timeNowMSec, SokBridgeIDCode *out_idCode)
{
	//genera un idCode random, univoco
	while (1)
	{
		out_idCode->data.buffer[0] = 1 + (u8)rhea::randomU32(254);
		out_idCode->data.buffer[1] = 1 + (u8)rhea::randomU32(254);
		out_idCode->data.buffer[2] = 1 + (u8)rhea::randomU32(254);
		out_idCode->data.buffer[3] = 1 + (u8)rhea::randomU32(254);

		assert(out_idCode->data.buffer[0] > 0 && out_idCode->data.buffer[1] > 0 && out_idCode->data.buffer[2] > 0 && out_idCode->data.buffer[3] > 0);
		if (u32MAX == priv_findByIDCode(*out_idCode))
			break;
	}

	return priv_createHandle(timeNowMSec, *out_idCode);
}

//*********************************************************
HSokBridgeClient& IdentifiedClientList::priv_createHandle (u64 timeNowMSec, const SokBridgeIDCode &idCode)
{
	//genero un nuovo handle
	nextHandle.index++;
	if (nextHandle.index == u16MAX)
		nextHandle.index = 0;

	//Creo il nuovo record
	u32 n = list.getNElem();
	list[n].handle = nextHandle;
	list[n].currentWebSocketHandleAsU32 = u32MAX;
	list[n].idCode = idCode;
	list[n].clientVer.zero();
	//list[n].hasPendingRequest = 0;
	list[n].timeCreatedMSec = timeNowMSec;
	list[n].lastTimeRcvMSec = timeNowMSec;

	return list[n].handle;
}

//*********************************************************
void IdentifiedClientList::purge(u64 timeNowMSec)
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (u32MAX == list(i).currentWebSocketHandleAsU32)
		{
			if (timeNowMSec - list(i).lastTimeRcvMSec > 60000)
			{
				list.removeAndSwapWithLast(i);
				i--;
				n--;
			}
		}
	}
}

//*********************************************************
void IdentifiedClientList::updateClientVerInfo(const HSokBridgeClient &handle, const SokBridgeClientVer &v)
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return;
	}

	list[i].clientVer = v;
}

//*********************************************************
bool IdentifiedClientList::compareClientVerInfo (const HSokBridgeClient &handle, const SokBridgeClientVer &v) const
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return false;
	}

	return (list(i).clientVer == v);
}

//*********************************************************
bool IdentifiedClientList::bindSocket (const HSokBridgeClient &handle, const HSokServerClient &hSok, u64 timeNowMSec)
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return false;
	}

	//devo bindarmi ad uno slot che attualmente non sia già bindato
	if (list(i).currentWebSocketHandleAsU32 == u32MAX)
	{
		list[i].currentWebSocketHandleAsU32 = hSok.asU32();
		list[i].lastTimeRcvMSec = timeNowMSec;
		return true;
	}

	DBGBREAK;
	return false;
}

//*********************************************************
void IdentifiedClientList::unbindSocket(const HSokBridgeClient &handle, u64 timeNowMSec UNUSED_PARAM)
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return;
	}

	assert(list(i).currentWebSocketHandleAsU32 != u32MAX);
	list[i].currentWebSocketHandleAsU32 = u32MAX;
}


