#include "DA3.h"
#include "../rheaCommonLib/rheaUtils.h"


//*********************************************
DA3::DA3()
{
	allocator = NULL;
	blob = NULL;
	sizeOfBlob = 0;
}

//*********************************************
void DA3::free()
{
	if (allocator && blob)
	{
		RHEAFREE(allocator, blob);
	}
	allocator = NULL;
	blob = NULL;
	sizeOfBlob = 0;
}

//*********************************************
bool DA3::loadInMemory(rhea::Allocator *allocatorIN, const char *fullFilePathAndName)
{
	free();

	sizeOfBlob = 0;
	blob = rhea::fs::fileCopyInMemory (fullFilePathAndName, allocatorIN, &sizeOfBlob);
	if (NULL == blob)
		return false;
	allocator = allocatorIN;
	return true;
}

//*********************************************
void DA3::save(const char *fullFilePathAndName)
{
	if (NULL == blob)
		return;

	const u32 SIZE = 1024;
	FILE *f = fopen(fullFilePathAndName, "wb");
	if (NULL == f)
		return;

	u32 fileSize = sizeOfBlob;
	u32 ct = 0;
	while (fileSize >= SIZE)
	{
		fwrite(&blob[ct], SIZE, 1, f);
		ct += SIZE;
		fileSize -= SIZE;
	}
	if (fileSize)
		fwrite(&blob[ct], fileSize, 1, f);
	fclose(f);
}

//*********************************************
u8 DA3::readU8(u32 location) const
{
	if (NULL == blob) return 0;
	if (location >= sizeOfBlob) return 0;
	return blob[location];
}

//*********************************************
void DA3::writeU8(u32 location, u8 value)
{
	if (NULL == blob) return;
	if (location >= sizeOfBlob) return;
	blob[location] = value;
}

//*********************************************
u16	DA3::readU16(u32 location) const
{
	if (NULL == blob) return 0;
	if (location >= sizeOfBlob-1) return 0;
	return rhea::utils::bufferReadU16_LSB_MSB(&blob[location]);
}

//*********************************************
void DA3::writeU16(u32 location, u16 value)
{
	if (NULL == blob) return;
	if (location >= sizeOfBlob - 1) return;
	rhea::utils::bufferWriteU16_LSB_MSB(&blob[location], value);
}

