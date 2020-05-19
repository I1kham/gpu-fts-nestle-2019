#include "rheaLogger.h"
#include "rhea.h"

using namespace rhea;


//****************************************
LoggerContext::LoggerContext()
{
    bufferSize = 0;
    bufferMaxSize = 1024;
    buffer = (char*)RHEAALIGNEDALLOC(rhea::getSysHeapAllocator(), bufferMaxSize, __alignof(char*));
}

//****************************************
LoggerContext::~LoggerContext()
{
	RHEAFREE(rhea::getSysHeapAllocator(), buffer);
}

//****************************************
void LoggerContext::_begin (Logger *owner, u32 channel)
{
    this->owner = owner;
    this->channel = channel;
    buffer[0] = 0;
    bufferSize = 0;
}

//****************************************
LoggerContext& LoggerContext::append (const LoggerContextEOL &eol UNUSED_PARAM)
{
    append ("\n", 1);
    owner->_flush (this);
    return *this;
}


//****************************************
LoggerContext& LoggerContext::append (const char *text, u32 lenInByte)
{
    if (NULL == text || lenInByte==0 || (NULL!=text && text[0]==0))
        return *this;

    if (bufferSize + lenInByte >= bufferMaxSize)
    {
        while (bufferSize + lenInByte >= bufferMaxSize)
            bufferMaxSize += 1024;
        RHEAFREE(rhea::getSysHeapAllocator(), buffer);
        buffer = (char*)RHEAALLOC(rhea::getSysHeapAllocator(), bufferMaxSize);
    }

    memcpy (&buffer[bufferSize], text, lenInByte);
    bufferSize += lenInByte;
    buffer[bufferSize] = 0;
    return *this;
}


