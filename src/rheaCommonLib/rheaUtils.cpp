#include "rheaUtils.h"
#include "rheaString.h"
#include "rheamd5.h"

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

u16 utils::Crc16_calc(const uint8_t* data, const size_t len)
/* calcolo del CRC (Cyclic Redundancy Check) su un buffer di lunghezza definita
parametri:
	data - puntatore al buffer da analizzare
	len - lunghezza del buffer
valore restituito:
	valore del CRC */
{
	size_t		i, j;
	uint16_t    crc = 0;

	for (i = 0; i < len; i++)
	{
		crc ^= (data[i] << 8);              /* data at top end, not bottom */
		for (j = 0; j < 8; j++)
		{
			if ((crc & 0x8000) == 0x8000)   /* top bit, not bottom */
				crc = (crc << 1) ^ 0x0589;  /* shift left, not right */
			else
				crc <<= 1;                  /* shift left, not right */
		}
	}

	return crc;
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

//******************************************************************************
bool rhea::utils::md5 (char *out, size_t sizeOfOutInBytes, const void *in, u32 length)
{
    if (sizeOfOutInBytes < 32)
        return false;

    MD5 md5 = MD5(in, length);
    md5.hexdigest(out, sizeOfOutInBytes);
    return true;
}

//******************************************************************************
u32 rhea::utils::copyBufferAsMuchAsYouCan (void *dst, u32 sizeOfDest, const void *src, u32 numBytesToCopyMax)
{
    if (sizeOfDest >= numBytesToCopyMax)
    {
        memcpy (dst, src, numBytesToCopyMax);
        return numBytesToCopyMax;
    }

    memcpy (dst, src, sizeOfDest);
    return sizeOfDest;
}
