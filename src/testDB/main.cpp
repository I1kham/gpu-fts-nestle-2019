#ifdef WIN32
#include <conio.h>
#endif
#include "../rheaCommonLib/rhea.h"
#include "../rheaDB/SQLite3/SQLInterface_SQLite.h"

void printErrAndWaitKey(const char *err)
{
    printf("%s", err);
#ifdef WIN32
    _getch();
#endif
}

//*****************************************************
bool open_or_createDB (rhea::SQLInterface &db, const char *filename)
{
	char s[512];
	sprintf_s (s, sizeof(s), "%s/%s", rhea::getPhysicalPathToAppFolder(), filename);

	return db.openDB(s);
}

//*****************************************************
bool create_table(rhea::SQLInterface &db)
{
	const char *sql = "CREATE TABLE COMPANY("  \
						"ID INT PRIMARY KEY     NOT NULL," \
						"NAME           TEXT    NOT NULL," \
						"AGE            INT     NOT NULL," \
						"ADDRESS        CHAR(50)," \
						"SALARY         REAL );";
	return db.exec(sql);
}

//*****************************************************
void test1()
{
	rhea::SQLInterface_SQLite db;

	if (!open_or_createDB(db, "test1.db3"))
	{
		printErrAndWaitKey("test1::err 1\n"); 
		return;
	}

	if (!create_table (db))
	{
		printErrAndWaitKey("test1::err 2\n");
		return;
	}

	printErrAndWaitKey("test1::OK\n");
}

//*****************************************************
void testInsert()
{
	rhea::SQLInterface_SQLite db;
	if (!open_or_createDB(db, "test1.db3"))
	{
		printErrAndWaitKey("test2::err 1\n");
		return;
	}

	const char *sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
		"VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
		"VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
		"VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
		"VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
	db.exec(sql);

}



//*****************************************************
void test2()
{
	rhea::SQLInterface_SQLite db;

	if (!open_or_createDB(db, "test1.db3"))
	{
		printErrAndWaitKey("test2::err 1\n");
		return;
	}

	rhea::SQLRst rst;
	rst.setup(rhea::memory_getDefaultAllocator());


	if (!db.q("SELECT SALARY, NAME, AGE, ADDRESS, ID FROM COMPANY ORDER BY SALARY, AGE", &rst))
		printf("test2:: query failed");
	else
	{
		u16 numCol = rst.getColsCount();
		printf("NUM COL:%d\n", numCol);
		for (u16 i = 0; i < numCol; i++)
			printf("  COLNAME-%02d: %s\n", (i + 1), rst.getColName(i));

		
		rhea::SQLRst::Row row;
		u16 numRow = rst.getRowCount();
		printf("\nNUM ROW:%d\n", numRow);
		rst.getRow(0, &row);

		for (u16 i2 = 0; i2 < numRow; i2++)
		{
			printf("  -------------------------------------- ROW %02d\n", i2 + 1);
			for (u16 i = 0; i < numCol; i++)
				printf("  VALUE-%02d: %s\n", (i + 1), rst.getValue(row, i));

			rst.nextRow(row);
		}
	}


#ifdef WIN32
	_getch();
#endif
	
}

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
	rhea::init("rheaSMU", &hInst);
#else
	rhea::init("rheaSMU", NULL);
#endif

    //test1();
    //testInsert();
	test2();
	
    rhea::deinit();
	return 0;
}


