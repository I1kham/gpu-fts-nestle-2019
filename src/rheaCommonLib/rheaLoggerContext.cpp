#include "rheaLogger.h"
#include "rheaMemory.h"

using namespace rhea;


//****************************************
LoggerContext::LoggerContext()
{
    bufferSize = 0;
    bufferMaxSize = 1024;
    buffer = (char*)RHEAALIGNEDALLOC(memory_getDefaultAllocator(), bufferMaxSize, __alignof(char*));
}

//****************************************
LoggerContext::~LoggerContext()
{
	RHEAFREE(memory_getDefaultAllocator(), buffer);
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
        RHEAFREE(memory_getDefaultAllocator(), buffer);
        buffer = (char*)RHEAALLOC(memory_getDefaultAllocator(), bufferMaxSize);
    }

    memcpy (&buffer[bufferSize], text, lenInByte);
    bufferSize += lenInByte;
    buffer[bufferSize] = 0;
    return *this;
}


