#ifndef _SQLInterface_h_
#define _SQLInterface_h_
#include "../rheaCommonLib/rhea.h"

namespace rhea
{
	class SQLRst;

	/******************************************************************************
	 * SQLInterface
	 *
	 */
	class SQLInterface
	{
	public:
							SQLInterface()											{}
		virtual				~SQLInterface()											{}

		virtual	bool		openDB(const char *filename) = 0;
		virtual void		closeDB() = 0;
		virtual bool		q (const char *query, SQLRst *out_result) = 0;
		virtual bool		exec (const char *sql) = 0;
	};


	/******************************************************************************
	 * SQLRst
	 *
	 */
	class SQLRst
	{
	public:
		struct Row
		{
			u32	rowOffset;
		};

	public:
						SQLRst ();
						~SQLRst()												{ unsetup(); }

		void			setup (rhea::Allocator *allocator, u32 startMemoryAllocationKb = 4);
		void			unsetup();


						//le seguenti sono da usarsi per interrogare i dati quando il recordset
						//è già popolato
		u16				getColsCount() const									{ return numCols; }
		const char*		getColName(u16 i) const;

		u16				getRowCount() const										{ return numRows; }
		
		bool			getRow (u16 i, Row *out_row) const;
		const char*		getValue (const Row &row, u16 iCol) const;
		bool			nextRow (Row &row) const;


		u8*				blobToString(rhea::Allocator *allocator, u32 *out_sizeInBytes) const;
						/*	NB: blobToString() è la versione comoda di:
								1- blob_calcMemoryNeeded()
								2- alloca dst buffer
								3- blob_copyToString (dstBuffer)

						
							Copia l'intero contenuto del recordset in una "stringa". Tutti i campi sono separati dal carattere §.
							La stringa viene allocata tramite [allocator] ed è compito del chiamante di farne il free.
							L'esatto numero di bytes utilizzati viene ritornato in [out_sizeInBytes] anche se non è necessariamente == alla dimensione allocata per
							la stringa (tipicamente la stringa sarà un po' più grande).
							La stringa è formattata in questo modo:
								nRow§nCol§colNam0§colNam1§...§colNameN§row1Value0§row1Value1§..§row1ValueN§row2Value0§..row2ValueN§rowNValue0§...rowNValueN

							Tutto è in formato testo, quindi anche [nRow] non è un intero ma è una stringa che rappresenta un numero.
							Eventuali valori NULL sono rappresentati dalla stringa NULL
						*/

		u32				blob_calcMemoryNeeded() const;
						/* ritorna una stima del numero di bytes minimi necessari per mettere tutto il recordset in una stringa (vedi blobToString)
						*/

		u32				blob_copyToString(u8 *dest, u32 sizeOfDest) const;
						/*	copia l'intero rst dentro [dst] che si suppone essere già stato allocato dal chiamante.
							Ritorna l'esatto numero di bytes utilizzati
						*/


						//le seguenti sono da utilizzarsi (in rigoroso ordine), per popolare
						//il recordset
		void			populate_begin();
		void				populate_beginColumnInfo (u16 numCols);
		void					populate_setNextColumnName(const char *colName);
		void				populate_endColumnInfo();

		void				populate_beginRow();
		void					populate_setNextColumnValue(const char *colValue);
		void				populate_endRow();
		void			populate_end();
	
	private:
		void			priv_growBlob();

	private:
		rhea::Allocator	*localAllocator;
		u8				*blob;
		u32				sizeOfBlob;
		u16				numRows;
		u16				numCols;
		u32				offsetForRowsIndex;
		u32				ct;
		u32				offsetOfFirstRow;
		u32				offsetOfCurRow;
		u16				curColIndex;
	};
} //namespace rhea
#endif