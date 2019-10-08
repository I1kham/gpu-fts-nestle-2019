#ifndef _SQLInterface_SQLite_h_
#define _SQLInterface_SQLite_h_
#include "../SQLInterface.h"
#include "sqlite3.h"


/***********************************************************
 NB: per compilare sotto windows/MSVC il file sqlite3.c, devi cliccare col destro sul file e impostare le seguenti cose (vedi https://stackoverflow.com/questions/27089460/compiling-sqlite-with-visual-studio-c-2013-throws-error-for-c-file)

	First, I had to change file's (ONLY FILE) property. 
	Right-click on the file and select properties, under the C/C++, select Advanced and then select Compile As and set it to C (neither default nor C++).

	Then, you should make sure that your .c file is compiled without clr. 
	Well, to do that, under the same C/C++ set of menu, select "Common Langugae Runtime Support" and set it to No Support....

***********************************************************/



namespace rhea
{
	class SQLInterface_SQLite : public SQLInterface
	{
	public:
						SQLInterface_SQLite();
						~SQLInterface_SQLite();

		bool			openDB(const char *filename);
		void			closeDB();
		bool			q (const char *query, SQLRst *out_result);
		bool			exec(const char *sql);

	private:
		sqlite3			*db;
	};
} //namespace rhea

#endif // _SQLInterface_SQLite_h_