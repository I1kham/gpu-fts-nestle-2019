#include "rheaUtils.h"
#include "rheaString.h"

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
void utils::dumpBufferInASCII (FILE *f, const u8 *buffer, u32 lenInBytes)
{
	for (u32 i = 0; i < lenInBytes; i++)
	{
		if (buffer[i] >= 32 && buffer[i] <= 126)
			fprintf(f, "%c", (char)buffer[i]);
		else
		{
			char hex[4];
			string::format::Hex8(buffer[i], hex, sizeof(hex));
			fprintf(f, "[%s]", hex);
		}		
	}
}