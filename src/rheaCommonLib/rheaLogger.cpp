#include "OS/OS.h"
#include "rheaLogger.h"
#include "rheaDateTime.h"
#include "rheaCriticalSection.h"


using namespace rhea;

const LoggerContextEOL	Logger::EOL;

#define ENTER_CRITICAL_SECTION  if (bEnabledThreadSafety) rhea::criticalsection::enter(cs)
#define LEAVE_CRITICAL_SECTION  if (bEnabledThreadSafety) rhea::criticalsection::leave(cs);



//****************************************
Logger::Logger(bool bEnableThreadSafety)
{
    memset (target, 0, sizeof(target));
    bEnabledThreadSafety = bEnableThreadSafety;

    if (bEnabledThreadSafety)
        rhea::criticalsection::init(&cs);
}

//****************************************
Logger::~Logger()
{
    if (bEnabledThreadSafety)
        rhea::criticalsection::close(cs);
}


//****************************************
void Logger::addTarget (ILogTarget *t)
{
    ENTER_CRITICAL_SECTION;

    for (u32 i=0; i<MAX_TARGET; i++)
    {
        if (target[i] == NULL)
        {
            target[i] = t;
            LEAVE_CRITICAL_SECTION;
            return;
        }
    }
    LEAVE_CRITICAL_SECTION;
    DBGBREAK;
}

//****************************************
void Logger::removeTarget (const ILogTarget *t)
{
    ENTER_CRITICAL_SECTION;

    u32 iLast = u32MAX;
    for (u32 i=0; i<MAX_TARGET; i++)
    {
        if (target[i] == NULL)
            break;
        iLast = i;
    }

    for (u32 i=0; i<MAX_TARGET; i++)
    {
        if (target[i] == t)
        {
            if (i == iLast)
                target[i] = NULL;
            else
            {
                target[i] = target[iLast];
                target[iLast] = NULL;
            }

            LEAVE_CRITICAL_SECTION;
            return;
        }
    }

    LEAVE_CRITICAL_SECTION;
    DBGBREAK;
}

//****************************************
LoggerContext& Logger::custom (u32 channel, bool bLogDate, bool bLogTime, const char *prefix, const char *message)
{
    char str[128];
    str[0] = 0;
    strcat_s (str, sizeof(str), prefix);

    rhea::DateTime dt;
    dt.setNow();
    if (bLogDate)
    {
        char temp[16];
        sprintf_s (temp, sizeof(temp), "%04d/%02d/%02d ", dt.date.getYear(), dt.date.getMonth(), dt.date.getDay());
        strcat_s (str, sizeof(str), temp);
    }


    if (bLogTime)
    {
        char temp[16];
        sprintf_s (temp, sizeof(temp), "%02d:%02d:%02d ", dt.time.getHour(), dt.time.getMin(), dt.time.getSec());
        strcat_s (str, sizeof(str), temp);
    }


    ENTER_CRITICAL_SECTION;

    ctx._begin (this, channel);
    ctx.append (str, (u32)strlen(str));
    ctx.append (message, (u32)strlen(message));

    return ctx;
}

//****************************************
void Logger::_flush (LoggerContext *ctx)
{
    for (u32 i=0; i<MAX_TARGET; i++)
    {
        if (NULL == target[i])
            break;
        target[i]->doLog (ctx->getChannel(), ctx->getMsg());
    }

#ifdef _DEBUG
    if (ctx->getChannel() == Logger::CHANNEL_ERR)
        DBGBREAK;
#endif

    LEAVE_CRITICAL_SECTION;

}
