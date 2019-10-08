#ifndef _DBList_h_
#define _DBList_h_
#include "SocketBridgeEnumAndDefine.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaDB/SQLInterface.h"

namespace socketbridge
{
    /***************************************************
     *	DBList
     *
     *  Mantiene una lista di db (e relativi handle) che sono attualmente aperti
     */
    class DBList
    {
    public:
							DBList()									{ }
							~DBList()									{ unsetup(); }

        void				setup (rhea::Allocator *allocator);
        void				unsetup();

		void				purge(u64 timeNowMSec);
							//elimina i db aperti che non sono stati utilizzati negli ultimi 5 minuti

		u16					getOrCreateDBHandle(u64 timeNowMSec, const char *fullFilePathAndName);
							//ritorna 0 se non è possibile aprire il DB

		bool				q (u16 dbHandle, u64 timeNowMSec, const char *sql, rhea::SQLRst *out_result);
		bool				exec (u16 dbHandle, u64 timeNowMSec, const char *sql);

    private:
		struct sEntry
		{
			rhea::SQLInterface	*db;
			char				*fullFilePathAndName;
			u16					dbHandle;
			u64					lastTimeUsedMSec;
		};

	private:
		void					priv_freeResouce(sEntry *s);
		u32						priv_findByDBHandle(u16 dbHanle) const;

	private:
		rhea::FastArray<sEntry>		list;
		u16							nextHandle;
	};
} // namespace socketbridge
#endif // _DBList_h_
