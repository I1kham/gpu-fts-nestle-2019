#include "IdentifiedClientList.h"

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
const IdentifiedClientList::sInfo* IdentifiedClientList::isKwnownSocket (const HSokServerClient &h) const
{
	u32 i = priv_findByHSokServerClient(h);
	if (u32MAX == i)
		return NULL;
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
const IdentifiedClientList::sInfo* IdentifiedClientList::isKnownIDCode (const SokBridgeIDCode &idCode) const
{
	u32 i = priv_findByIDCode(idCode);
	if (u32MAX == i)
		return NULL;
	return &list(i);
}

//*********************************************************
HSokBridgeClient& IdentifiedClientList::newEntry (const SokBridgeIDCode &idCode, u64 timeNowMSec)
{
	//idCode deve essere univoco
	assert(u32MAX == priv_findByIDCode(idCode));

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
	list[n].timeCreatedMSec = timeNowMSec;
	list[n].lastTimeConnectedMSec = 0;
	list[n].lastTimeDisconnectedMSec = 0;

	return list[n].handle;
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
void IdentifiedClientList::bindSocket (const HSokBridgeClient &handle, const HSokServerClient &hSok, u64 timeNowMSec)
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return;
	}

	assert(list(i).currentWebSocketHandleAsU32 == u32MAX);
	list[i].currentWebSocketHandleAsU32 = hSok.asU32();
	list[i].lastTimeConnectedMSec = timeNowMSec;
}

//*********************************************************
void IdentifiedClientList::unbindSocket(const HSokBridgeClient &handle, u64 timeNowMSec)
{
	u32 i = priv_findByHSokBridgeClient(handle);
	if (u32MAX == i)
	{
		DBGBREAK;
		return;
	}

	assert(list(i).currentWebSocketHandleAsU32 != u32MAX);
	list[i].currentWebSocketHandleAsU32 = u32MAX;
	list[i].lastTimeDisconnectedMSec = timeNowMSec;
}


