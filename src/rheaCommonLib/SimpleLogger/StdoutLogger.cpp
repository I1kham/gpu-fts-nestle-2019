#include "StdoutLogger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

using namespace rhea;


//*************************************************
StdoutLogger::StdoutLogger()
{
	OSCriticalSection_init(&cs);
	indent = isANewLine = 0;
    priv_buildIndentStr();
	
}

//*************************************************
StdoutLogger::~StdoutLogger() 
{
	OSCriticalSection_close(cs);
}

//*************************************************
void StdoutLogger::priv_buildIndentStr()
{
    u32 n = indent*2;
    if (n > MAX_INDENT_CHAR)
        n = MAX_INDENT_CHAR;

    memset (strIndent, ' ', n);
    strIndent[n] = 0;
}

//*************************************************
void StdoutLogger::priv_out (const char *what)
{
    if (isANewLine)
    {
        fprintf (stdout, "%s", strIndent);
        isANewLine = 0;
    }

	fflush(stdout);
    fprintf (stdout, "%s", what);
}

//*************************************************
void StdoutLogger::incIndent() 
{ 
	OSCriticalSection_enter(cs);
	++indent; 
	priv_buildIndentStr();
	OSCriticalSection_leave(cs);
}

//*************************************************
void StdoutLogger::decIndent() 
{
	OSCriticalSection_enter(cs);
	if (indent) 
		--indent; 
	priv_buildIndentStr();
	OSCriticalSection_leave(cs);
}


//*************************************************
void StdoutLogger::log (const char *format, ...)
{
	OSCriticalSection_enter(cs);
	
	va_list argptr;
    va_start(argptr, format);
    vsnprintf (buffer, INTERNAL_BUFFER_SIZE, format, argptr);
    va_end(argptr);

    u32 n = strlen(buffer);
	if (n == 0)
	{
		OSCriticalSection_leave(cs);
		return;
	}


	
	
	u32 i = 0;
    if (buffer[0] == '\n')
    {
        fprintf (stdout, "\n");
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
                priv_out (&buffer[iStart]);
            fprintf (stdout, "\n");

            isANewLine = 1;
            i++;
            iStart = i;
            continue;
        }

        if (buffer[i] == 0x00)
        {
            if (i-iStart)
                priv_out (&buffer[iStart]);
            break;
        }

        ++i;
    }

	OSCriticalSection_leave(cs);
}


