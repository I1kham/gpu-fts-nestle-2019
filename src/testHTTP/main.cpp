#ifdef WIN32
#include <conio.h>
#endif
#include "../rheaCommonLib/rhea.h"
#include "miniz.h"
#include "../rheaCommonLib/compress/rheaCompress.h"


//*****************************************************
void waitKB()
{
	_getch();
}


//**********************************
void zip_test1()
{
	rhea::Allocator *allocator = rhea::getScrapAllocator();

	const char pSrc[] = { "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." \
	  "Good morning Dr. Chandra. This is Hal. I am ready for my first lesson." };

	const u8 COMPRESSION_LEVEL = 8;
	mz_ulong	compressedSize = 4 * 1024 * 1024;
	u8 *pCompressed = (u8*)RHEAALLOC (allocator, compressedSize);
	if (Z_OK != mz_compress2 (pCompressed, &compressedSize, (const u8*)pSrc, strlen((const char*)pSrc), COMPRESSION_LEVEL))
    {
		printf("compress() failed!\n");
		return;
    }
	printf("Compressed from %d to %d bytes\n", strlen((const char*)pSrc), (mz_uint32)compressedSize);


	mz_ulong	decompressedSize = 4 * 1024 * 1024;
	u8 *pDecompressed = (u8*)RHEAALLOC (allocator, decompressedSize);
	if (Z_OK != mz_uncompress (pDecompressed,&decompressedSize, pCompressed, compressedSize))
    {
		printf("decompress() failed!\n");
		return;
    }
	printf("Decompressed from %d to %d bytes\n", compressedSize, decompressedSize);

	if (memcmp (pSrc, pDecompressed, strlen((const char*)pSrc)) != 0)
		printf("ERROR: decompressed and src does not match\n");

	RHEAFREE(allocator, pCompressed);
	RHEAFREE(allocator, pDecompressed);
}

void zip_test2()
{
	rhea::CompressUtility cu;

	cu.begin ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\zipt.rheazip", 8);
	cu.addFile ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\current\\gui\\web\\img\\animationRound.gif", (const u8*)"aa_animationRound.gif");
	cu.addFile ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\current\\gui\\web\\upload\\Drink_direct_water.png", (const u8*)"pippo\\pluto\\aa_Drink_direct_water.png");
	cu.end();


	rhea::CompressUtility::decompresAll ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\zipt.rheazip", (const u8*)"c:\\vuota");
}

void zip_test3()
{
	rhea::CompressUtility::decompresAll ((const u8*)"C:\\Users\\giallanon\\Desktop\\aaa\\web\\mobile.rheazip", (const u8*)"c:\\vuota\\aaa");
	return;

	rhea::CompressUtility cu;

	cu.begin ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\zip3.rheazip", 8);

	cu.excludeFolder ((const u8*)"E:\\wwwroot\\rhea\\Fusion2\\web\\mobile\\fonts");
	cu.addFilesInFolder ((const u8*)"E:\\wwwroot\\rhea\\Fusion2\\web\\mobile", (const u8*)"", true);
	cu.end();

	rhea::CompressUtility::decompresAll ((const u8*)"E:\\rhea\\gpu-fts-nestle-2019\\bin\\zip3.rheazip", (const u8*)"c:\\vuota\\zip3");

	
}

//*****************************************************
int main()
{
#ifdef WIN32
	HINSTANCE hInst = NULL;
	rhea::init("testHTTP", &hInst);
#else
	rhea::init("testHTTP", NULL);
#endif
	
	//zip_test1(); waitKB();
	//zip_test2(); waitKB();
	zip_test3();

    rhea::deinit();
	return 0;
}


