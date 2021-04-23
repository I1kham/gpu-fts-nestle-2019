#include "rheaCompress.h"
#include "../rheaString.h"
#include "../rheaUtils.h"

using namespace rhea;


//********************************************
CompressUtility::CompressUtility()
{
	fHeader = NULL;
	fData = NULL;
	tempBuffer = NULL;
	sizeOfTempBuffer = 0;
	utf8_fullDstFileNameAndPath = NULL;
	localAllocator = rhea::getSysHeapAllocator();
	excludedFolderList.setup (localAllocator, 8);
}

//********************************************
CompressUtility::~CompressUtility()
{
	end();
}

//********************************************
void CompressUtility::begin (const u8* utf8_fullDstFileNameAndPath, u8 compressionLevel)
{
	this->numFiles = 0;
	this->compressionLevel = compressionLevel;
	this-> utf8_fullDstFileNameAndPath = rhea::string::utf8::allocStr (localAllocator, utf8_fullDstFileNameAndPath);
	u8 s[1024];

    string::utf8::spf (s, sizeof(s), "%s.header", utf8_fullDstFileNameAndPath);
	fHeader = rhea::fs::fileOpenForWriteBinary (s);

	sprintf_s ((char*)s, sizeof(s), "%s.data", utf8_fullDstFileNameAndPath);
	fData = rhea::fs::fileOpenForWriteBinary (s);

	sizeOfTempBuffer = 4 * 1024 * 1024;
    tempBuffer = RHEAALLOCT(u8*,localAllocator, sizeOfTempBuffer);
}

//********************************************
bool CompressUtility::addFile (const u8* utf8_fullFileNameAndPath, const u8* utf8_fullDestFileNameAndPath)
{
	u32 srcSize = 0;
	u8 *src = fs::fileCopyInMemory(utf8_fullFileNameAndPath, localAllocator, &srcSize);
	if (NULL != src)
	{
		const u32 estimatedCompressedSize = compressCalcEstimatedCompressedSize(srcSize);
		if (estimatedCompressedSize > sizeOfTempBuffer)
		{
			RHEAFREE(localAllocator, tempBuffer);
			sizeOfTempBuffer = estimatedCompressedSize;
            tempBuffer = RHEAALLOCT(u8*,localAllocator, sizeOfTempBuffer);
		}

		u32 sizeCompressed = sizeOfTempBuffer;
		if (compressFromMemory (src, srcSize, compressionLevel, tempBuffer, &sizeCompressed))
		{
            const u32 position = static_cast<u32>(::ftell (fData));
			fs::fileWrite (fData, tempBuffer, sizeCompressed);

			const u8 *pSanitizedFullDestiFilenameAndPath = NULL;
			u8 sanitizedFullDestiFilenameAndPath[1024];
			rhea::fs::sanitizePath (utf8_fullDestFileNameAndPath, sanitizedFullDestiFilenameAndPath, sizeof(sanitizedFullDestiFilenameAndPath));
			if (sanitizedFullDestiFilenameAndPath[0] == '/')
				pSanitizedFullDestiFilenameAndPath = &sanitizedFullDestiFilenameAndPath[1];
			else
				pSanitizedFullDestiFilenameAndPath = sanitizedFullDestiFilenameAndPath;


			//header
			//4 byte per la lunghezza del filename
			//4 byte per la posizione in fData
			//4 byte per la dimensione del file non compressa
			//4 byte per la dimensione del file compressa
			//n byte con il nome del file
			const u32 nameLen = rhea::string::utf8::lengthInBytes (pSanitizedFullDestiFilenameAndPath);
			rhea::utils::bufferWriteU32 (tempBuffer, nameLen);
			rhea::utils::bufferWriteU32 (&tempBuffer[4], position);
			rhea::utils::bufferWriteU32 (&tempBuffer[8], srcSize);
			rhea::utils::bufferWriteU32 (&tempBuffer[12], sizeCompressed);
			memcpy (&tempBuffer[16], pSanitizedFullDestiFilenameAndPath, nameLen);
			fs::fileWrite (fHeader, tempBuffer, 16+nameLen);

			++numFiles;
		}

		RHEAFREE(localAllocator, src);

		return true;
	}

	return false;
}

//********************************************
void CompressUtility::end()
{
	if (NULL == fHeader)
		return;
	fclose (fHeader);
	fclose (fData);
	excludedFolderList.reset();

	u8 s[1024];
	sprintf_s ((char*)s, sizeof(s), "%s.header", utf8_fullDstFileNameAndPath);
	fHeader = rhea::fs::fileOpenForReadBinary (s);


	//fusione di fHeader e fData
	FILE *fFinal = rhea::fs::fileOpenForWriteBinary (utf8_fullDstFileNameAndPath);

	//header
	{
		//magic str: 7 byte
		u32 ct = 0;
		tempBuffer[ct++] = 'r';
		tempBuffer[ct++] = 'h';
		tempBuffer[ct++] = 'e';
		tempBuffer[ct++] = 'a';
		tempBuffer[ct++] = 'z';
		tempBuffer[ct++] = 'i';
		tempBuffer[ct++] = 'p';
		fwrite (tempBuffer, 7, 1, fFinal);

		//versione:	2 byte
		rhea::utils::bufferWriteU16 (tempBuffer, 0x0001);
		fwrite (tempBuffer, 2, 1, fFinal);

		//num of files: 4byte
		rhea::utils::bufferWriteU32 (tempBuffer, numFiles);
		fwrite (tempBuffer, 4, 1, fFinal);

		//start of data-sector: 4byte
		const u32 startOfDataSector = 17 + (u32)fs::filesize(fHeader);
		rhea::utils::bufferWriteU32 (tempBuffer, startOfDataSector);
		fwrite (tempBuffer, 4, 1, fFinal);
	}

	//copio fHeader che contiene l'elenco dei filename e gli offset
	fs::fileCopyInChunkWithPreallocatedBuffer (fHeader, (u32)fs::filesize(fHeader), fFinal, tempBuffer, sizeOfTempBuffer);
	fclose (fHeader);

	//copio fData
	sprintf_s ((char*)s, sizeof(s), "%s.data", utf8_fullDstFileNameAndPath);
	fData = rhea::fs::fileOpenForReadBinary (s);
	fs::fileCopyInChunkWithPreallocatedBuffer (fData, (u32)fs::filesize(fData), fFinal, tempBuffer, sizeOfTempBuffer);
	fclose (fData);
	fclose(fFinal);

	//delete dei 2 file temporanei
	sprintf_s ((char*)s, sizeof(s), "%s.header", utf8_fullDstFileNameAndPath);
	fs::fileDelete (s);

	sprintf_s ((char*)s, sizeof(s), "%s.data", utf8_fullDstFileNameAndPath);
	fs::fileDelete (s);


	RHEAFREE(localAllocator, utf8_fullDstFileNameAndPath);
	RHEAFREE(localAllocator, tempBuffer);
	utf8_fullDstFileNameAndPath = NULL;
	tempBuffer = NULL;
	fHeader = NULL;
	fData = NULL;
}

//********************************************
void CompressUtility::excludeFolder (const u8* utf8_fullSRCFolderPathNoSlash)
{
	const u32 len = rhea::string::utf8::lengthInBytes (utf8_fullSRCFolderPathNoSlash);
	if (len)
	{
		u32 n = excludedFolderList.getNElem();
		excludedFolderList[n] = utf8_fullSRCFolderPathNoSlash;
		excludedFolderList[n].sanitizePath();
	}
}

//********************************************
bool CompressUtility::addFilesInFolder (const u8* utf8_fullSRCFolderPathNoSlash, const u8* utf8_fullDSTFolderNameAndPathNoSlash, bool bRecurseSubFolder)
{
	rhea::utf8::String s1, s2;
	s1.prealloc(512);
	s2.prealloc(512);

	OSFileFind ff;
	if (!fs::findFirst (&ff, utf8_fullSRCFolderPathNoSlash, (const u8*)"*.*"))
		return false;

	//per prima cosa tutti i file del folder attuale
	do
	{
		const u8 *fname = fs::findGetFileName(ff);
		if (fname[0] == '.')
			continue;
		if (fs::findIsDirectory(ff))
			continue;

		s1 = utf8_fullSRCFolderPathNoSlash;
		s1 <<  "/";
		s1 << fname;

		s2 = utf8_fullDSTFolderNameAndPathNoSlash;
		s2 << "/";
		s2 << fname;
		this->addFile (s1.getBuffer(), s2.getBuffer());

	} while (fs::findNext(ff));
	fs::findClose(ff);

	//se necessario, ricorsione sulle sotto directory
	if (!bRecurseSubFolder)
		return true;

	fs::findFirst (&ff, utf8_fullSRCFolderPathNoSlash, (const u8*)"*.*");
	do
	{
		const u8 *fname = fs::findGetFileName(ff);
		if (fname[0] == '.')
			continue;
		if (!fs::findIsDirectory(ff))
			continue;

		s1 = utf8_fullSRCFolderPathNoSlash;
		s1 <<  "/";
		s1 << fname;
		s1.sanitizePath();

		//Eventuali folder da escludere
		for (u32 i = 0; i < excludedFolderList.getNElem(); i++)
		{
			if (excludedFolderList(i).isEqualTo (s1, true))
			{
				s1 = "";
				break;
			}
		}

		if (s1.lengthInBytes() > 0)
		{
			s2 = utf8_fullDSTFolderNameAndPathNoSlash;
			s2 << "/";
			s2 << fname;
			this->addFilesInFolder (s1.getBuffer(), s2.getBuffer(), true);
		}
	} while (fs::findNext(ff));
	fs::findClose(ff);
	return true;
}

//********************************************
bool CompressUtility::decompresAll (const u8* utf8_fullSRCFileNameAndPath, const u8* pathDestNoSlash)
{
	FILE *f = fs::fileOpenForReadBinary (utf8_fullSRCFileNameAndPath);
	if (NULL == f)
		return false;

	u32 numFiles = 0;
	u32 startOfDataSector = 0;
	
	//decodifico header
	{
		//magicstr
		u8 buffer[64];
		fread (buffer, 7, 1, f);
		if (memcmp (buffer, "rheazip", 7) != 0)
		{
			fclose(f);
			return false;
		}

		//versione
		fread (buffer, 2, 1, f);
		const u16 versione = rhea::utils::bufferReadU16(buffer);
		if (versione != 0x0001)
		{
			fclose(f);
			return false;
		}

		//num files
		fread (buffer, 4, 1, f);
		numFiles = rhea::utils::bufferReadU32(buffer);

		//start of data sector
		fread (buffer, 4, 1, f);
		startOfDataSector = rhea::utils::bufferReadU32(buffer);
	}

	u32 SIZE_OF_BUFFER1 = 1024 * 1024;
	u32 SIZE_OF_BUFFER2 = 1024 * 1024;
    u8 *buffer1 = RHEAALLOCT(u8*,rhea::getScrapAllocator(), SIZE_OF_BUFFER1);
    u8 *buffer2 = RHEAALLOCT(u8*,rhea::getScrapAllocator(), SIZE_OF_BUFFER2);
	while (numFiles--)
	{
		//4 byte per la lunghezza del filename
		fread (buffer1, 4, 1, f);
		const u32 fNameLen = rhea::utils::bufferReadU32(buffer1);
			
		//4 byte per la posizione in fData
		fread (buffer1, 4, 1, f);
		const u32 dataPosInFile = startOfDataSector + rhea::utils::bufferReadU32(buffer1);
		
		//4 byte per la dimensione del file non compressa
		fread (buffer1, 4, 1, f);
		const u32 fileLen = rhea::utils::bufferReadU32(buffer1);

		//4 byte per la dimensione del file compressa
		fread (buffer1, 4, 1, f);
		const u32 compressedFileLen = rhea::utils::bufferReadU32(buffer1);

		//n byte con il nome del file
		fread (buffer1, fNameLen, 1, f);
		buffer1[fNameLen] = 0;

		//memorizzo l'attuale posizione in f
		const u32 lastArchiveFilePos = ftell(f);


		//creo il nuovo file
		//Nel caso in cui il path preveda delle directory, creo anche quelle se necessario
		sprintf_s ((char*)buffer2, SIZE_OF_BUFFER2, "%s/%s", pathDestNoSlash, buffer1);
		fs::extractFilePathWithOutSlash (buffer2, buffer1, SIZE_OF_BUFFER1);
		fs::folderCreate (buffer1);
		FILE *fDST = fs::fileOpenForWriteBinary(buffer2);
		if (fDST)
		{
			//devo leggere in memoria la versione compressa
			if (compressedFileLen > SIZE_OF_BUFFER1)
			{
				RHEAFREE(rhea::getScrapAllocator(), buffer1);
				SIZE_OF_BUFFER1 = compressedFileLen;
                buffer1 = RHEAALLOCT(u8*,rhea::getScrapAllocator(), SIZE_OF_BUFFER1);
			}
			fseek (f, dataPosInFile, SEEK_SET);
			fs::fileRead (f, buffer1, compressedFileLen);

			//decomprimo
			if (fileLen > SIZE_OF_BUFFER2)
			{
				RHEAFREE(rhea::getScrapAllocator(), buffer2);
				SIZE_OF_BUFFER2 = fileLen;
                buffer2 = RHEAALLOCT(u8*,rhea::getScrapAllocator(), SIZE_OF_BUFFER2);
			}
			
			u32 sizeOfUncompressed = SIZE_OF_BUFFER2;
			if (rhea::decompressFromMemory (buffer1, compressedFileLen, buffer2, &sizeOfUncompressed))
				fs::fileWrite (fDST, buffer2, sizeOfUncompressed);
			fclose(fDST);
		}

		fseek (f, lastArchiveFilePos, SEEK_SET);
	}
	fclose (f);
	RHEAFREE(rhea::getScrapAllocator(), buffer1);
	RHEAFREE(rhea::getScrapAllocator(), buffer2);
	return true;
}
