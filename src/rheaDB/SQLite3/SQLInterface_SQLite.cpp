#include <stdio.h>
#include "SQLInterface_SQLite.h"


using namespace rhea;

//*************************************************************** 
SQLInterface_SQLite::SQLInterface_SQLite()
{
	db = NULL;
}

//*************************************************************** 
SQLInterface_SQLite::~SQLInterface_SQLite()
{
	closeDB();
}

//*************************************************************** 
bool SQLInterface_SQLite::openDB(const char *filename)
{
	int err = sqlite3_open(filename, &db);
	if (SQLITE_OK == err)
		return true;

	closeDB();
	return false;
}

//*************************************************************** 
void SQLInterface_SQLite::closeDB()
{
	if (NULL == db)
		return;
	sqlite3_close(db);
	db = NULL;
}

//*************************************************************** 
bool SQLInterface_SQLite::exec(const char *sql)
{
	if (NULL == db)
		return false;

	int err = sqlite3_exec(db, sql, NULL, NULL, NULL);
	return (err == 0);
}

//*************************************************************** 
bool SQLInterface_SQLite::q(const char *query, SQLRst *out_result)
{
	if (NULL == db)
		return false;


    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2 (db, query, strlen(query), &stmt, NULL);
	if (rc != SQLITE_OK) 
	{
		//printf("error: ", sqlite3_errmsg(db));
		return false;
	}

	out_result->populate_begin();

	//nomi delle colonne
	int nCols = sqlite3_column_count (stmt);
	out_result->populate_beginColumnInfo(nCols);
	for (int i = 0; i < nCols; i++)
	{
		const char *colName = sqlite3_column_name(stmt, i);
		out_result->populate_setNextColumnName(colName);
		//printf("COLNAME-%02d: %s\n", i, colName);
	}
	out_result->populate_endColumnInfo();

	//righe
	while ( (rc = sqlite3_step(stmt)) == SQLITE_ROW) 
	{
		out_result->populate_beginRow();

		for (int i = 0; i < nCols; i++)
		{
			const unsigned char *val = sqlite3_column_text(stmt, i);
			//printf("COL-%02d: %s\n", (i+1), val);
			out_result->populate_setNextColumnValue((const char*)val);
		}

		out_result->populate_endRow();

	}
	
	bool ret = true;
	if (rc != SQLITE_DONE) 
	{
		//print("error: ", sqlite3_errmsg(db));
		ret = false;
	}
	sqlite3_finalize(stmt);
	out_result->populate_end();

	return ret;
}
