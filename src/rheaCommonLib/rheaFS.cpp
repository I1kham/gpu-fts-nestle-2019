#include <limits.h>
#include "rhea.h"
#include "rheaString.h"


using namespace rhea;

/****************************************************
 * rimuove eventuali . e .. e doppi /
 */
void fs::sanitizePath(const char *path, char *out_sanitizedPath, u32 sizeOfOutSanitzed UNUSED_PARAM)
{
	if (NULL == path)
	{
		out_sanitizedPath[0] = 0;
		return;
	}
	if (path[0] == 0x00)
	{
		out_sanitizedPath[0] = 0;
		return;
	}

    strcpy_s (out_sanitizedPath, sizeOfOutSanitzed, path);
	sanitizePathInPlace(out_sanitizedPath);
}

void fs::sanitizePathInPlace(char *path)
{
	const u32 nBytesToCheck = (u32)strlen(path);
	for (u32 i = 0; i < nBytesToCheck; i++)
	{
		if (path[i] == '\\')
			path[i] = '/';
	}

	u32 i = 0, t = 0;
	while (i < nBytesToCheck)
	{
		if (path[i] == '\\')
			path[i] = '/';

		if (path[i] == '/')
		{
			path[t++] = path[i++];
			while (path[i] == '/')
				++i;
		}
		else if (path[i] == '.')
		{
			//se xxx/./yyy
			if (i > 0 && path[i - 1] == '/' && path[i + 1] == '/')
				i += 2;
			//se xxx/../yyy
			else if (i > 0 && path[i - 1] == '/' && path[i + 1] == '.')
			{
				i += 3;
				if (t >= 2)
					t -= 2;
				while (t && path[t] != '/')
					--t;
				if (path[t] == '/')
					++t;
			}
			else
				path[t++] = path[i++];
		}
		else
			path[t++] = path[i++];
	}
	path[t] = 0;
	if (t > 1 && path[t - 1] == '/')
		path[t - 1] = 0;
}


//**************************************************************************
void fs::extractFileExt(const char *filename, char *out, u32 sizeofout)
{
	assert(out && sizeofout >= 3);
	out[0] = 0;

	u32 len = (u32)strlen(filename);
	if (len > 0)
	{
		u32 i = len;
		while (i-- > 0)
		{
			if (filename[i] == '.')
			{
				if (i < len - 1)
				{
					u32 numBytesToCopy = len - i - 1;
					if (numBytesToCopy >= sizeofout - 1)
					{
						DBGBREAK;
						numBytesToCopy = sizeofout - 2;
					}
					memcpy(out, &filename[i + 1], numBytesToCopy);
					out[numBytesToCopy] = 0;
				}
				return;
			}
		}
	}
}

//**************************************************************************
void fs::extractFileNameWithExt(const char *filename, char *out, u32 sizeofout)
{
	assert(out && sizeofout >= 3);
	out[0] = 0;

	u32 len = (u32)strlen(filename);
	if (len > 0)
	{
		u32 i = len;
		while (i-- > 0)
		{
			if (filename[i] == '/' || filename[i] == '\\')
			{
				u32 numBytesToCopy = len - i - 1;
				if (numBytesToCopy >= sizeofout - 1)
				{
					DBGBREAK;
					numBytesToCopy = sizeofout - 2;
				}
				memcpy(out, &filename[i + 1], numBytesToCopy);
				out[numBytesToCopy] = 0;
				return;
			}
		}

		u32 numBytesToCopy = len;
		if (numBytesToCopy >= sizeofout - 1)
		{
			DBGBREAK;
			numBytesToCopy = sizeofout - 2;
		}
		memcpy(out, filename, numBytesToCopy);
		out[numBytesToCopy] = 0;
		return;
	}
}

//**************************************************************************
void fs::extractFileNameWithoutExt(const char *filename, char *out, u32 sizeofout)
{
	fs::extractFileNameWithExt(filename, out, sizeofout);

	u32 len = (u32)strlen(out);
	while (len--)
	{
		if (out[len] == '.')
		{
			out[len] = 0;
			return;
		}
	}
}

//**************************************************************************
void fs::extractFilePathWithSlash(const char *filename, char *out, u32 sizeofout)
{
	assert(out && sizeofout >= 3);
	out[0] = 0;

	u32 len = (u32)strlen(filename);
	while (len-- > 0)
	{
		if (filename[len] == '/' || filename[len] == '\\')
		{
			u32 numBytesToCopy = len + 1;
			if (numBytesToCopy >= sizeofout)
			{
				DBGBREAK;
				numBytesToCopy = sizeofout - 1;
			}
			memcpy(out, filename, numBytesToCopy);
			out[numBytesToCopy] = 0;
			fs::sanitizePathInPlace(out);
			return;
		}
	}
}

//**************************************************************************
void fs::extractFilePathWithOutSlash(const char *filename, char *out, u32 sizeofout)
{
	assert(out && sizeofout >= 3);
	out[0] = 0;

	u32 len = (u32)strlen(filename);
	while (len-- > 0)
	{
		if (filename[len] == '/' || filename[len] == '\\')
		{
			u32 numBytesToCopy = len;
			if (numBytesToCopy >= sizeofout)
			{
				DBGBREAK;
				numBytesToCopy = sizeofout - 1;
			}
			memcpy(out, filename, numBytesToCopy);
			out[numBytesToCopy] = 0;
			fs::sanitizePathInPlace(out);
			return;
		}
	}
}


//**************************************************************************
void fs::findComposeFullFilePathAndName(const OSFileFind &ff, const char *pathNoSlash, char *out, u32 sizeofOut)
{
	sprintf_s(out, sizeofOut, "%s/", pathNoSlash);

	const u32 n = strlen(out);
	fs::findGetFileName(ff, &out[n], sizeofOut - n);
}

//**************************************************************************
void fs::deleteAllFileInFolderRecursively(const char *pathSenzaSlash, bool bAlsoRemoveFolder)
{
	if (!folderExists(pathSenzaSlash))
		return;

	OSFileFind ff;
	if (fs::findFirst(&ff, pathSenzaSlash, "*.*"))
	{
		do
		{
			char s[512];
			findComposeFullFilePathAndName(ff, pathSenzaSlash, s, sizeof(s));

			if (fs::findIsDirectory(ff))
			{
				const char *fname = fs::findGetFileName(ff);
				if (fname[0] != '.')
					fs::deleteAllFileInFolderRecursively(s, true);
			}
			else
				fs::fileDelete(s);
		} while (fs::findNext(ff));
		fs::findClose(ff);
	}	

	if (bAlsoRemoveFolder)
		fs::folderDelete(pathSenzaSlash);
}

//**************************************************************************
u64 fs::filesize(FILE *fp)
{
	long prev = ftell(fp);
	fseek(fp, 0L, SEEK_END);
	long sz = ftell(fp);
	fseek(fp, prev, SEEK_SET);
	return sz;
}

//**************************************************************************
bool fs_do_open_and_copy_fileCopy (const char *srcFullFileNameAndPath, const char *dstFullFileNameAndPath, void *buffer, u32 BUFFER_SIZE)
{
	FILE *fSRC = fopen(srcFullFileNameAndPath, "rb");
	if (NULL == fSRC)
		return false;

	FILE *fDST = fopen(dstFullFileNameAndPath, "wb");
	if (NULL == fDST)
	{
		fclose(fSRC);
		return false;
	}

	u64 fLen = fs::filesize(fSRC);
	while (fLen >= BUFFER_SIZE)
	{
		fread(buffer, BUFFER_SIZE, 1, fSRC);
		fwrite(buffer, BUFFER_SIZE, 1, fDST);
		fLen -= BUFFER_SIZE;
	}
	if (fLen)
	{
		fread (buffer, (size_t)fLen, 1, fSRC);
		fwrite(buffer, (size_t)fLen, 1, fDST);
	}
	fclose(fSRC);
	fclose(fDST);
	return true;
}

//**************************************************************************
bool fs::fileCopy (const char *srcFullFileNameAndPath, const char *dstFullFileNameAndPath)
{
	const u32 BUFFER_SIZE = 4096;
	rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
	void *buffer = RHEAALLOC(allocator, BUFFER_SIZE);
	bool ret = fs_do_open_and_copy_fileCopy(srcFullFileNameAndPath, dstFullFileNameAndPath, buffer, BUFFER_SIZE);
	RHEAFREE(allocator, buffer);
	return ret;
}

//**************************************************************************
bool fs_folderCopy_with_buffer (const char *srcFullPathNoSlash, const char *dstFullPathNoSlash, void *buffer, u32 BUFFER_SIZE)
{
	if (!fs::folderCreate(dstFullPathNoSlash))
	{
		DBGBREAK;
		return false;
	}

	if (!fs::folderExists(srcFullPathNoSlash))
		return false;

	bool ret = true;
	OSFileFind ff;
	if (fs::findFirst(&ff, srcFullPathNoSlash, "*.*"))
	{
		do
		{
			if (fs::findIsDirectory(ff))
			{
				const char *dirname = fs::findGetFileName(ff);
				if (dirname[0] != '.')
				{
					char src[1024], dst[1024];
					sprintf_s(src, sizeof(src), "%s/%s", srcFullPathNoSlash, dirname);
					sprintf_s(dst, sizeof(dst), "%s/%s", dstFullPathNoSlash, dirname);
					if (!fs_folderCopy_with_buffer(src, dst, buffer, BUFFER_SIZE))
						ret = false;
				}
			}
			else
			{
				const char *fname = fs::findGetFileName(ff);
				
				char src[1024], dst[1024];
				sprintf_s(src, sizeof(src), "%s/%s", srcFullPathNoSlash, fname);
				sprintf_s(dst, sizeof(dst), "%s/%s", dstFullPathNoSlash, fname);
				fs_do_open_and_copy_fileCopy(src, dst, buffer, BUFFER_SIZE);
			}
		} while (fs::findNext(ff));
		fs::findClose(ff);
	}

	return ret;
}


//**************************************************************************
bool fs::folderCopy(const char *srcFullPathNoSlash, const char *dstFullPathNoSlash)
{
	//alloco un buffer per il file copy
	const u32 BUFFER_SIZE = 4096;
	rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
	
	void *buffer = RHEAALLOC(allocator, BUFFER_SIZE);

	bool ret = fs_folderCopy_with_buffer(srcFullPathNoSlash, dstFullPathNoSlash, buffer, BUFFER_SIZE);

	RHEAFREE(allocator, buffer);
	return ret;

}

//*********************************************
bool fs::doesFileNameMatchJolly (const char *strFilename, const char *strJolly)
{
    assert (NULL != strFilename && NULL != strJolly);
    string::parser::Iter iterFilename;
    iterFilename.setup (strFilename, 0, (u32)strlen(strFilename));

    string::parser::Iter iterJolly;
    iterJolly.setup (strJolly, 0, (u32)strlen(strJolly));

    while (1)
    {
        if (iterJolly.getCurChar() == 0x00 || iterFilename.getCurChar() == 0x00)
        {
            if (iterJolly.getCurChar() == 0x00 && iterFilename.getCurChar() == 0x00)
                return true;
            return false;
        }


        if (iterJolly.getCurChar() == '?')
        {
            //il char jolly è ?, quindi va bene un char qualunque
            iterFilename.next();
            iterJolly.next();
        }
        else if (iterJolly.getCurChar() == '*')
        {
            //il char jolly è un *, quindi prendo il prox char jolly e lo cerco nel filename
            iterJolly.next();
            if (iterJolly.getCurChar() == 0x00)
                return true;

            //cerco il char jolly
            while (1)
            {
                iterFilename.next();
                if (iterFilename.getCurChar() == 0x00)
                    return false;
                if (iterFilename.getCurChar() == iterJolly.getCurChar())
                {
                    if (fs::doesFileNameMatchJolly (iterFilename.getCurStrPointer(), iterJolly.getCurStrPointer()))
                        return true;
                }
            }
        }
        else
        {
            //il carattere jolly è un char normale, quindi deve essere uguale al char del filename
            if (iterFilename.getCurChar() != iterJolly.getCurChar())
                return false;
            iterFilename.next();
            iterJolly.next();
        }

    }
    return true;
}


//*********************************************
u8* fs::fileCopyInMemory(const char *srcFullFileNameAndPath, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer)
{
	FILE *f = fopen(srcFullFileNameAndPath, "rb");
	if (NULL == f)
	{
		*out_sizeOfAllocatedBuffer = 0;
		return NULL;
	}

	u8 *ret = fs::fileCopyInMemory(f, allocator, out_sizeOfAllocatedBuffer);
	fclose(f);
	return ret;
}


//*********************************************
u8* fs::fileCopyInMemory (FILE *f, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer)
{
	u32 fsize = (u32)fs::filesize(f);

	*out_sizeOfAllocatedBuffer = (u32)fsize;
	u8 *buffer = (u8*)RHEAALLOC(allocator, (u32)fsize);
	if (NULL == buffer)
	{
		*out_sizeOfAllocatedBuffer = 0;
		return NULL;
	}

	u32 CHUNK = 1024 * 1024;
	u32 ct = 0;
	while (fsize >= CHUNK)
	{
		fread(&buffer[ct], CHUNK, 1, f);
		fsize -= CHUNK;
		ct += CHUNK;
	}

	if (fsize)
		fread(&buffer[ct], fsize, 1, f);

	return buffer;

}
