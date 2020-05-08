#ifdef WIN32
#include <conio.h>
#endif
#include "../rheaCommonLib/rhea.h"
#include "../rheaDB/SQLite3/SQLInterface_SQLite.h"


//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
	rhea::init("testHTTP", &hInst);
#else
	rhea::init("testHTTP", NULL);
#endif



    rhea::deinit();
	return 0;
}


