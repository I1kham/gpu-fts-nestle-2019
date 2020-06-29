#ifndef _rheaFileLogger_h_
#define _rheaFileLogger_h_
#include "ISimpleLogger.h"
#include "../rhea.h"

namespace rhea
{
    /******************************************
     * FileLogger
     *
     * Semplice log con funzionalità di indentazione che butta
     * tutto su un file
     */
    class FileLogger : public ISimpleLogger
    {
    public:
                            FileLogger(const u8* const fullFilePathAndName);
        virtual             ~FileLogger();

		void                incIndent();
		void                decIndent();
        void                log (const char *format, ...);


    private:
        static const u16    MAX_INDENT_CHAR = 31;
        static const u16    INTERNAL_BUFFER_SIZE = 1024;

    private:
        void                priv_buildIndentStr();
        void                priv_out (FILE *f, const char *what);

    private:
        u8                  fullFilePathAndName[256];
        u16                 indent;
        char                strIndent[MAX_INDENT_CHAR+1];
        char                buffer[INTERNAL_BUFFER_SIZE];
        u8                  isANewLine;
		OSCriticalSection	cs;

    };
} //namespace rhea
#endif //_rheaStdoutLogger_h_
