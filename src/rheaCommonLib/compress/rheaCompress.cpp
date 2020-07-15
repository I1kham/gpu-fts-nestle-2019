#include "rheaCompress.h"
#include "miniz.h"


using namespace rhea;

//**********************************************************
u32 rhea::compressCalcEstimatedCompressedSize (u32 srcLenInBytes)
{
	return (u32)compressBound(srcLenInBytes);
}

//**********************************************************
bool rhea::compressFromMemory (const u8 *src, u32 numBytesToCompress, u8 compressionLevel, u8 *dst, u32 *in_out_sizeOfDst)
{
	mz_ulong compressedSize = (*in_out_sizeOfDst);
	if (Z_OK == mz_compress2 (dst, &compressedSize, src, numBytesToCompress, compressionLevel))
    {
		*in_out_sizeOfDst = (u32)compressedSize;
		return true;
    }	
	return false;
}

//**********************************************************
bool rhea::compressFromMemory (const u8 *src, u32 numBytesToCompress, u8 compressionLevel, rhea::Allocator *allocator, u8 *out_dst, u32 *out_compressedSize)
{
	//stima dei byte richiesti dalla versione compressa
	*out_compressedSize = compressCalcEstimatedCompressedSize(numBytesToCompress);
	out_dst = RHEAALLOCT(u8*,allocator, (*out_compressedSize));
	if (!rhea::compressFromMemory(src, numBytesToCompress, compressionLevel, out_dst, out_compressedSize))
	{
		RHEAFREE(allocator, out_dst);
		*out_compressedSize = 0;
		return false;
	}
	return true;
}

//**********************************************************
bool rhea::decompressFromMemory (const u8 *compressedSRC, u32 numBytesCompressed, u8 *out_decompressed, u32 *in_out_sizeOfDecompressed)
{
	mz_ulong decompressedSize = (*in_out_sizeOfDecompressed);
	if (Z_OK != mz_uncompress (out_decompressed, &decompressedSize, compressedSRC, numBytesCompressed))
	{
		*in_out_sizeOfDecompressed = 0;
		return false;
	}

	*in_out_sizeOfDecompressed = (u32)decompressedSize;
	return true;
}