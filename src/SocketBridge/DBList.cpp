#include "DBList.h"
#include "../rheaDB/SQLite3/SQLInterface_SQLite.h"

using namespace socketbridge;


//*********************************************************
void DBList::setup (rhea::Allocator *allocator)
{
	nextHandle = 1;
    list.setup (allocator, 16);
}


//*********************************************************
void DBList::unsetup()
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		priv_freeResouce(&list[i]);
	}
    list.unsetup();
}

//*********************************************************
void DBList::priv_freeResouce(sEntry *s)
{
	if (s->fullFilePathAndName)
		RHEAFREE(list.getAllocator(), s->fullFilePathAndName);
	if (s->db)
	{
		s->db->closeDB();
		RHEADELETE(list.getAllocator(), s->db);
	}
}

//*********************************************************
void DBList::purge(u64 timeNowMSec UNUSED_PARAM)
{
	/*	il concetto di purge() è corretto.
	Il problema è che se una GUI si collega e apre il DB, e poi per 5 minuti non fa nulla, finische che chiudo
	la connessione al DB anche se la GUI è ancora collegata. Quando poi questa dopo 10 minuti decide di fare una query,
	trova il DB chiuso perchè l'ho purged().
	Bisogna quindi mantenere una serie di identifiedClient associata ad ogni connessione DB e fare il purge() dei DB che non hanno
	più identifiedClient attivi associati.
	Non si considerano più i 5 minuti quindi, ma la connessione DB rimane viva fintanto che anche uno solo dei client che la sta usando è viva.
	Questa cosa richiedere un po' di tempo, la farò in futuro.
	Per il momento la connessione DB la lascio sempre aperta...
	*/
	/*
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (timeNowMSec - list(i).lastTimeUsedMSec >= 5*60000)
		{
			priv_freeResouce(&list[i]);
			list.removeAndSwapWithLast(i);
			--i;
			--n;
		}
	}
	*/
}

//*********************************************************
u16 DBList::getOrCreateDBHandle(u64 timeNowMSec, const char *fullFilePathAndName)
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (strcasecmp(fullFilePathAndName, list(i).fullFilePathAndName) == 0)
		{
			list[i].lastTimeUsedMSec = timeNowMSec;
			return list(i).dbHandle;
		}
	}

	rhea::SQLInterface	*db = RHEANEW(list.getAllocator(), rhea::SQLInterface_SQLite)();
	if (db)
	{
		if (db->openDB(fullFilePathAndName))
		{
			list[n].lastTimeUsedMSec = timeNowMSec;
			list[n].fullFilePathAndName = rhea::string::alloc(list.getAllocator(), fullFilePathAndName);
			list[n].dbHandle = nextHandle++;
			list[n].db = db;
			return list[n].dbHandle;
		}
		RHEADELETE(list.getAllocator(), db);
	}
	return 0;
}

//*********************************************************
u32 DBList::priv_findByDBHandle(u16 dbHandle) const
{
	u32 n = list.getNElem();
	for (u32 i = 0; i < n; i++)
	{
		if (list(i).dbHandle == dbHandle)
			return i;
	}
	return u32MAX;
}


//*********************************************************
bool DBList::q(u16 dbHandle, u64 timeNowMSec, const char *sql, rhea::SQLRst *out_result)
{
	u32 index = priv_findByDBHandle(dbHandle);
	if (u32MAX == index)
		return false;
	
	list[index].lastTimeUsedMSec = timeNowMSec;
	return list(index).db->q(sql, out_result);
}

//*********************************************************
bool DBList::exec(u16 dbHandle, u64 timeNowMSec, const char *sql)
{
	u32 index = priv_findByDBHandle(dbHandle);
	if (u32MAX == index)
		return false;

	list[index].lastTimeUsedMSec = timeNowMSec;
	return list(index).db->exec(sql);
}
