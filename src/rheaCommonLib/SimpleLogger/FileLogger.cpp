#include "FileLogger.h"
#include "../rheaCriticalSection.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

using namespace rhea;


//*************************************************
FileLogger::FileLogger(const u8 *fullFilePathAndName)
{
	rhea::criticalsection::init(&cs);
    rhea::string::utf8::copyStr (this->fullFilePathAndName, sizeof(this->fullFilePathAndName), fullFilePathAndName);
    indent = isANewLine = 0;
    priv_buildIndentStr();

    FILE *f = rhea::fs::fileOpenForWriteText(this->fullFilePathAndName);
    {
        rhea::DateTime dt;
        dt.setNow();
        char dd[64];
        dt.formatAs_YYYYMMDDHHMMSS (dd, sizeof(dd), ' ', '-', ':');
        fprintf (f, "%s\n\n", dd);
        rhea::fs::fileClose(f);
    }
	
}

//*************************************************
FileLogger::~FileLogger()
{
	rhea::criticalsection::close(cs);
}

//*************************************************
void FileLogger::priv_buildIndentStr()
{
    u32 n = indent*2;
    if (n > MAX_INDENT_CHAR)
        n = MAX_INDENT_CHAR;

    memset (strIndent, ' ', n);
    strIndent[n] = 0;
}

//*************************************************
void FileLogger::priv_out (FILE *f, const char *what)
{
    if (isANewLine)
    {
        fprintf (f, "%s", strIndent);
        isANewLine = 0;
    }

    fprintf (f, "%s", what);
}

//*************************************************
void FileLogger::incIndent()
{ 
	rhea::criticalsection::enter(cs);
	++indent; 
	priv_buildIndentStr();
	rhea::criticalsection::leave(cs);
}

//*************************************************
void FileLogger::decIndent()
{
	rhea::criticalsection::enter(cs);
	if (indent) 
		--indent; 
	priv_buildIndentStr();
	rhea::criticalsection::leave(cs);
}


//*************************************************
void FileLogger::log (const char *format, ...)
{
	rhea::criticalsection::enter(cs);
	
	va_list argptr;
    va_start(argptr, format);
    vsnprintf (buffer, INTERNAL_BUFFER_SIZE, format, argptr);
    va_end(argptr);

    u32 n = strlen(buffer);
	if (n == 0)
	{
		rhea::criticalsection::leave(cs);
		return;
	}


    FILE *f = rhea::fs::fileOpenForAppendText(this->fullFilePathAndName);
	
	
	u32 i = 0;
    if (buffer[0] == '\n')
    {
        fprintf (f, "\n");
        isANewLine = 1;
        i++;
    }

    u32 iStart = i;
    while (1)
    {
        if (buffer[i] == '\n')
        {
            buffer[i] = 0;
            if (i-iStart)
                priv_out (f, &buffer[iStart]);
            fprintf (f, "\n");

            isANewLine = 1;
            i++;
            iStart = i;
            continue;
        }

        if (buffer[i] == 0x00)
        {
            if (i-iStart)
                priv_out (f, &buffer[iStart]);
            break;
        }

        ++i;
    }

    fflush(f);
    rhea::fs::fileClose (f);
    rhea::criticalsection::leave(cs);
}


