#include "SQLInterface.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace rhea;

/* L'intero recordset è memorizzato in un unico blob di memoria organizzato come segue:
	u16	offset of colname-0
	u16	offset of colname-1
	..
	u16	offset of colname-n
	n	colname-0							-> stringa con il nome della colonna
	u8	0x00					
		colname-1							-> stringa con il nome della colonna
		0x00
		...
		colname-n							-> stringa con il nome della colonna
		0x00
	u32 offsetOfNextRow						-> qui inizia la prima riga di dati. I primi 4 byte indicano l'offset assoluto della riga successiva
	u16	relative offset-of-row-value-0		-> ogni row ha n valori, uno per ogni colonna. Qui vegono memorizzato gli offset (relativi all'inizio della row)
	u16	relative offset-of-row-value-1		-> che indicano dove sono memorizzati i valori delle singole colonne di questa specifica row
	u16	relative offset-of-row-value-2
	..	val-col-0
		0x00
		val-col-1
		0x00
		...
		val-col-n
		0x00
	u32 offsetOfNextRow						-> qui inizia la seconda riga di dati
	....
	...
	u32 offsetOfNextRow						-> qui inizia l'ultima riga di dati. Il suo [offsetOfNextRow] viene messo a 0xffffffff
	...
	...
*/

//**********************************************************
SQLRst::SQLRst()
{
	localAllocator = NULL;
	blob = NULL;
	sizeOfBlob = 0;
}

//**********************************************************
void SQLRst::setup(rhea::Allocator *allocator, u32 startMemoryAllocationKb)
{
	localAllocator = allocator;
	
	//prealloco un po' di mem
	if (startMemoryAllocationKb == 0)
		startMemoryAllocationKb = 1;
	sizeOfBlob = startMemoryAllocationKb * 1024;
	blob = (u8*)RHEAALLOC(localAllocator, sizeOfBlob);
}

//**********************************************************
void SQLRst::unsetup()
{
	if (NULL == localAllocator)
		return;
	if (NULL != blob)
		RHEAFREE(localAllocator, blob);
	blob = NULL;
	sizeOfBlob = 0;
	localAllocator = NULL;
}

//**********************************************************
void SQLRst::priv_growBlob()
{
	u32 newSize = sizeOfBlob * 2;
	u8 *newBlob = (u8*)RHEAALLOC(localAllocator, newSize);
	memcpy(newBlob, blob, sizeOfBlob);
	RHEAFREE(localAllocator, blob);

	blob = newBlob;
	sizeOfBlob = newSize;
}

//**********************************************************
const char*	SQLRst::getColName(u16 i) const
{
	if (i >= numCols)
		return NULL;
	u16 offset = utils::bufferReadU16(&blob[i * 2]);
	return (const char*)&blob[offset];
}

//**********************************************************
void SQLRst::populate_begin()
{
	numRows = 0;
	numCols = 0;
	offsetForRowsIndex = 0;
	ct = 0;
	curColIndex = 0;
}

//**********************************************************
void SQLRst::populate_beginColumnInfo (u16 numColsIN)
{
	assert(ct == 0);
	numCols = numColsIN;

	//riservo spazio per gli offset delle col name
	ct += sizeof(u16) * numColsIN;
	
}

//**********************************************************
void SQLRst::populate_setNextColumnName (const char *colName)
{
	assert(curColIndex < numCols);
	assert(NULL != colName);
	assert(colName[0] != 0x00);
	
	u32 n = (u32)strlen(colName);
	if (ct + n + 1 >= sizeOfBlob)
		priv_growBlob();

	//scrivo l'offset di questa colonna
	utils::bufferWriteU16(&blob[curColIndex * 2], (u16)ct);

	//Scrivo il nome comprensivo di 0x00
	memcpy(&blob[ct], colName, n + 1);
	ct += (n + 1);
	
	curColIndex++;
}

//**********************************************************
void SQLRst::populate_endColumnInfo()
{
	assert(curColIndex == numCols);

	//voglio che la row inizi in memoria ad un multiplo di 4, per questioni di allineamento
	while (ct % 4 != 0)
		++ct;

	offsetOfFirstRow = ct;
	curColIndex = 0;

}

//**********************************************************
void SQLRst::populate_beginRow()
{
	++numRows;
	curColIndex = 0;
	offsetOfCurRow = ct;
	
	//lascio spazio per l'offset della prossima row
	ct += 4;

	//lascio spazio per gli offset dei valori di questra row
	ct += sizeof(u16) * this->numCols;
}

//**********************************************************
void SQLRst::populate_setNextColumnValue(const char *colValue)
{
	bool isNULL = false;
	if (NULL == colValue)
		isNULL = true;
	else if (colValue[0] == 0x00)
		isNULL = true;

	u32 n = 0;
	if (!isNULL)
		n = (u32)strlen(colValue);

	//alloco nuovo spazio se necessario
	if (ct + n + 1 >= sizeOfBlob)
		priv_growBlob();
		
	//scrivo l'offset di questa valore
	u32 pos = offsetOfCurRow + 4 + curColIndex*sizeof(u16);
	u32 relative_offset = ct - offsetOfCurRow;
	utils::bufferWriteU16(&blob[pos], (u16)relative_offset);

	//Scrivo il nome comprensivo di 0x00
	if (!isNULL)
		memcpy(&blob[ct], colValue, n + 1);
	else
		blob[ct] = 0x00;
	ct += (n + 1);

	++curColIndex;
}

//**********************************************************
void SQLRst::populate_endRow()
{
	assert(curColIndex == numCols);

	//l'offset della prossima row lo voglio allineato a 4
	while (ct % 4 != 0)
		++ct;

	//scrivo l'offset della prossima row
	utils::bufferWriteU32(&blob[offsetOfCurRow], ct);
}

//**********************************************************
void SQLRst::populate_end()
{
	//l'ultima colonna "Punta" a 0xffffffff cosi' posso sapere che è l'ultima
	if (numRows)
		utils::bufferWriteU32(&blob[offsetOfCurRow], 0xFFFFFFFF);
}


//**********************************************************
bool SQLRst::getRow (u16 i, Row *out_row) const
{
	if (i >= numRows)
		return false;

	u32 row_offset = offsetOfFirstRow;
	while (i--)
	{
		const u32 nextRowOffset = utils::bufferReadU32(&blob[row_offset]);
		row_offset = nextRowOffset;
	}

	out_row->rowOffset = row_offset;
	return true;
}

//**********************************************************
const char* SQLRst::getValue(const Row &row, u16 iCol) const
{
	if (iCol >= numCols)
		return NULL;

	const u32 rowOffset = row.rowOffset;
	u32 col_relative_offset = utils::bufferReadU16(&blob[rowOffset + 4 + sizeof(u16) * iCol]);
	const u32 absolute_offset = rowOffset + col_relative_offset;
	return (const char*)&blob[absolute_offset];
}

//**********************************************************
bool SQLRst::nextRow(Row &row) const
{
	const u32 nextRowOffset = utils::bufferReadU32(&blob[row.rowOffset]);
	if (u32MAX == nextRowOffset)
		return false;

	row.rowOffset = nextRowOffset;
	return true;
}

//**********************************************************
u32 SQLRst::blob_calcMemoryNeeded() const 
{ 
	return ct + getColsCount() * (3+getRowCount());
}

//**********************************************************
u8* SQLRst::blobToString(rhea::Allocator *allocator, u32 *out_sizeInBytes) const
{
	u32 nBytes = blob_calcMemoryNeeded();
	u8 *buffer = (u8*)RHEAALLOC(allocator, nBytes);

	*out_sizeInBytes = blob_copyToString(buffer, nBytes);
	return buffer;
}

//**********************************************************
u32 SQLRst::blob_copyToString(u8 *buffer, u32 sizeOfDest) const
{
	const u8 SEP0 = 0xC2; //il caratter § in UTF8 è rappresentato dai 2 byte 0xC2 0xA7
	const u8 SEP1 = 0xA7;


	//numRows § numCols
	u32 n = 0;
	char s[16];
	sprintf_s(s, sizeof(s), "%d", numRows);
	n = (u32)strlen(s);
	memcpy (buffer, s, n);

	buffer[n++] = SEP0;
	buffer[n++] = SEP1;

	sprintf_s(s, sizeof(s), "%d", numCols);
	{
		u32 i = (u32)strlen(s);
		memcpy(&buffer[n], s, i);
		n += i;
	}

	//colNames
	for (u16 i = 0; i < numCols; i++)
	{
		buffer[n++] = SEP0;
		buffer[n++] = SEP1;

		const char *name = getColName(i);
		u32 t = (u32)strlen(name);
		memcpy(&buffer[n], name, t);
		n += t;
	}

	//rows
	Row row;
	getRow(0, &row);
	for (u16 i2 = 0; i2 < numRows; i2++)
	{
		for (u16 i = 0; i < numCols; i++)
		{
			buffer[n++] = SEP0;
			buffer[n++] = SEP1;

			const char *value = getValue(row, i);
			if (NULL == value || value[0] == 0x00)
			{
				buffer[n++] = 'N';
				buffer[n++] = 'U';
				buffer[n++] = 'L';
				buffer[n++] = 'L';
			}
			else
			{
				u32 t = (u32)strlen(value);
				memcpy(&buffer[n], value, t);
				n += t;
			}
		}

		nextRow(row);
	}


	assert(n <= sizeOfDest);
	return n;
}
