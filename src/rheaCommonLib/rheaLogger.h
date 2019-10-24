#ifndef _rheaLogger_h_
#define _rheaLogger_h_
#include <stdio.h>
#include <string.h>
#include "OS/OS.h"
#include "rheaDataTypes.h"



namespace rhea
{
    //fwd declaration
    class Logger;


    /*===============================================================
     * Target per un logger
     *=============================================================*/
    class ILogTarget
    {
    public:
                        ILogTarget()									{ }
        virtual			~ILogTarget()									{ }

        virtual	void	doLog	(u32 channel, const char *msg) = 0;

    private:
                        RHEA_NO_COPY_NO_ASSIGN(ILogTarget);
    };


    /*===============================================================
     *
     *=============================================================*/
    class LoggerContextEOL
    {
    public:
                    LoggerContextEOL()			{ }
    };


    /*===============================================================
     *
     *=============================================================*/
    class LoggerContext
    {
    public:
                                LoggerContext();
                                ~LoggerContext();


                                //=============================================== append
                LoggerContext&	append (const char *msg, u32 lenInByte);
                LoggerContext&	append (int i)																							{ char buf[32]; sprintf_s (buf, sizeof(buf), "%d", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (unsigned int i)																					{ char buf[32]; sprintf_s (buf, sizeof(buf), "%u", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (long i)																							{ char buf[32]; sprintf_s (buf, sizeof(buf), "%ld", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (unsigned long i)																				{ char buf[32]; sprintf_s (buf, sizeof(buf), "%lu", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (long long i)                                                                                    { char buf[32]; sprintf_s (buf, sizeof(buf), "%lld", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (unsigned long long i)                                                                           { char buf[32]; sprintf_s (buf, sizeof(buf), "%llu", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (f32 i)																							{ char buf[32]; sprintf_s (buf, sizeof(buf), "%f", i); return append (buf, (u32)strlen(buf)); }
                LoggerContext&	append (const LoggerContextEOL &eol);

        friend	LoggerContext&	operator<<  (LoggerContext &logger, const char *msg)													{ return logger.append (msg, (u32)strlen(msg)); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, char c)                                                             { return logger.append (&c, 1); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, int i)																{ return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, unsigned int i)														{ return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, long i)																{ return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, unsigned long i)													{ return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, long long i)                                                        { return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, unsigned long long i)                                               { return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, f32 i)																{ return logger.append (i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, u8 i)																{ return logger.append ((unsigned int)i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, i8 i)																{ return logger.append ((unsigned int)i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, u16 i)																{ return logger.append ((unsigned int)i); }
        friend	LoggerContext&	operator<<  (LoggerContext &logger, i16 i)																{ return logger.append ((unsigned int)i); }
        friend	void			operator<<  (LoggerContext &logger, const LoggerContextEOL &eol)										{ logger.append (eol); }

                                //=============================================== query
                u32				getChannel() const																						{ return channel; }
                const char*		getMsg() const																							{ return buffer; }

                                //=============================================== uso interno
                void			_begin (Logger *owner, u32 channel);

    private:
                Logger			*owner;
                u32				channel;
                char			*buffer;
                u16				bufferSize, bufferMaxSize;
    };



    /*===============================================================
     * Logger. Logga su tutti i "LogTarget" in lista
     *=============================================================*/
    class Logger
    {
    public:
        static	const LoggerContextEOL	EOL;
        static	const u32				CHANNEL_ERR = 0;
        static	const u32				CHANNEL_WARN = 1;
        static	const u32				CHANNEL_LOG = 2;
        static	const u32				CHANNEL_INFO = 3;

    public:
                                Logger (bool bEnableThreadSafety);
                                ~Logger();

                                //===============================================
                void			addTarget (ILogTarget *target);
                void			removeTarget (const ILogTarget *target);

                                //===============================================
                LoggerContext&	log	(const char *message, bool bLogDate=false, bool bLogTime=true)										{ return custom (CHANNEL_LOG, bLogDate, bLogTime,  "        ", message); }
                LoggerContext&	logInfo (const char *message, bool bLogDate=false, bool bLogTime=true)                                  { return custom (CHANNEL_INFO, bLogDate, bLogTime, "[INFO]  ", message); }
                LoggerContext&	logWarn (const char *message, bool bLogDate=false, bool bLogTime=true)                                  { return custom (CHANNEL_WARN, bLogDate, bLogTime, "[WARN]  ", message); }
                LoggerContext&	logErr	(const char *message, bool bLogDate=false, bool bLogTime=true)                                  { return custom (CHANNEL_ERR, bLogDate, bLogTime,  "[ERR]   ", message); }
                LoggerContext&	logInfoPrefix (const char *prefix, const char *message, bool bLogDate=false, bool bLogTime=true)        { return custom (CHANNEL_INFO, bLogDate, bLogTime,  prefix, message); }
                LoggerContext&	custom (u32 channel, bool bLogDate, bool bLogTime, const char *prefix, const char *message);


                                //=============================================== uso interno
                void			_flush (LoggerContext *ctx);

    private:
        static const int		MAX_TARGET = 16;
        ILogTarget				*target[MAX_TARGET];
        LoggerContext			ctx;
        OSCriticalSection		cs;
        bool					bEnabledThreadSafety;
    };


};

#endif // _rheaLogger_h_
