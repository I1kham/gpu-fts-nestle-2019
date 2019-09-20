#include "rheaUtils.h"


using namespace rhea;

//*************************************************************************
u8 utils::simpleChecksum8_calc (const void *bufferIN, u32 lenInBytes)
{
    const u8 *buffer = (const u8*)bufferIN;
    u8 ret = 0;
    for (u32 i=0;i<lenInBytes;i++)
        ret += buffer[i];
    return ret;
}

//*************************************************************************
u16 utils::simpleChecksum16_calc (const void *bufferIN, u32 lenInBytes)
{
    const u8 *buffer = (const u8*)bufferIN;
    u16 ret = 0;
    for (u32 i=0;i<lenInBytes;i++)
        ret += buffer[i];
    return ret;
}


//*************************************************************************
u64 utils::filesize (FILE *fp)
{
	long prev = ftell (fp);
	fseek (fp, 0L, SEEK_END);
	long sz = ftell(fp);
	fseek (fp, prev, SEEK_SET);
	return sz;
}