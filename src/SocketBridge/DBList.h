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

		u16					getOrCreateDBHandle (u64 timeNowMSec, const u8* utf8_fullFilePathAndName);
							//ritorna 0 se non è possibile aprire il DB

		bool				q (u16 dbHandle, u64 timeNowMSec, const u8 * const utf8_sql, rhea::SQLRst *out_result);
		bool				exec (u16 dbHandle, u64 timeNowMSec, const u8 * const utf8_sql);
		
		void				closeDBByPath (const u8 * const utf8_fullFilePathAndName);
		void				closeDBByHandle(u16 dbHandle);

    private:
		struct sEntry
		{
			rhea::SQLInterface	*db;
			u8					*utf8_fullFilePathAndName;
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
