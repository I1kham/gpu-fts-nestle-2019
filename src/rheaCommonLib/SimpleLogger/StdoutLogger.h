#ifndef _rheaStdoutLogger_h_
#define _rheaStdoutLogger_h_
#include "ISimpleLogger.h"
#include "../rhea.h"

namespace rhea
{
    /******************************************
     * StdoutLogger
     *
     * Semplice log con funzionalità di indentazione che butta
     * tutto sullo stdout
     */
    class StdoutLogger : public ISimpleLogger
    {
    public:
                            StdoutLogger();
		virtual             ~StdoutLogger();

		void                incIndent();
		void                decIndent();
        void                log (const char *format, ...);


    private:
        static const u16    MAX_INDENT_CHAR = 31;
        static const u16    INTERNAL_BUFFER_SIZE = 1024;

    private:
        void                priv_buildIndentStr();
        void                priv_out (const char *what);

    private:
        u16                 indent;
        char                strIndent[MAX_INDENT_CHAR+1];
        char                buffer[INTERNAL_BUFFER_SIZE];
        u8                  isANewLine;
		OSCriticalSection	cs;

    };
} //namespace rhea
#endif //_rheaStdoutLogger_h_
