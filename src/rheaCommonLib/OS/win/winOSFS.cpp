#ifdef WIN32
#include "winOS.h"
#include "../../rhea.h"
#include "../../rheaString.h"


using namespace rhea;

//*****************************************************
bool platform::FS_DirectoryCreate (const char *path)
{
	char	temp[1024];
	string::parser::Iter src, value;
	src.setup(path);

	if (path[1] == ':')
	{
		src.next();
		src.next();
		src.next();
	}

	while (string::parser::advanceUntil(src, "/", 1))
	{
		src.prev();
		const char *s2 = src.getCurStrPointer();
		src.next();
		src.next();

		if (s2)
		{
			u32 n = (u32)(s2 - path + 1);
			memcpy_s(temp, sizeof(temp), path, n);
			temp[n] = 0;

			BOOL ret = ::CreateDirectory(temp, NULL);
			if (ret == 1)
				continue;
			if (GetLastError() == ERROR_ALREADY_EXISTS)
				continue;
			return false;
		}
	}

	BOOL ret = ::CreateDirectory(path, NULL);
	if (ret != 1 && GetLastError() != ERROR_ALREADY_EXISTS)
		return false;
	return true;
}

//*****************************************************
bool platform::FS_DirectoryDelete(const char *path)
{
	if (!::RemoveDirectory(path))
		return false;
	return true;

}

//*****************************************************
bool platform::FS_DirectoryExists(const char *path)
{
	DWORD ftyp = GetFileAttributes(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	return false;
}

//*****************************************************
bool platform::FS_fileExists(const char *filename)
{
	DWORD ftyp = GetFileAttributes(filename);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;
	if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return true;
	return false;
}

//*****************************************************
bool platform::FS_fileDelete(const char *filename)
{
	return (::DeleteFile(filename) != 0);
}

//*****************************************************
bool platform::FS_fileRename(const char *oldFilename, const char *newFilename)
{
	return (rename(oldFilename, newFilename) == 0);
}


//*****************************************************
bool platform::FS_findFirst(OSFileFind *ff, const char *strPathNoSlash, const char *strJolly)
{
	assert(ff->h == INVALID_HANDLE_VALUE);

	char filename[1024];
	sprintf_s(filename, sizeof(filename), "%s/%s", strPathNoSlash, strJolly);
	ff->h = FindFirstFile(filename, &ff->findData);
	if (ff->h == INVALID_HANDLE_VALUE)
		return false;
	return true;
}

//*****************************************************
bool platform::FS_findNext(OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	return (FindNextFile(ff.h, &ff.findData) == TRUE);
}

//*****************************************************
bool platform::FS_findIsDirectory(const OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	return ((ff.findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//*****************************************************
const char* platform::FS_findGetFileName(const OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	return ff.findData.cFileName;
}

//*****************************************************
void platform::FS_findGetFileName(const OSFileFind &ff, char *out, u32 sizeofOut)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	sprintf_s(out, sizeofOut, "%s", ff.findData.cFileName);
}

//*****************************************************
void platform::FS_findGetCreationTime(const OSFileFind &ff, rhea::DateTime *out_dt)
{
	assert(ff.h != INVALID_HANDLE_VALUE);

	SYSTEMTIME  stime, ltime;
	FileTimeToSystemTime(&ff.findData.ftCreationTime, &stime);
	SystemTimeToTzSpecificLocalTime(NULL, &stime, &ltime);

	out_dt->date.setYMD(ltime.wYear, ltime.wMonth, ltime.wDay);
	out_dt->time.setHMS(ltime.wHour, ltime.wMinute, ltime.wSecond, 0);
}

//*****************************************************
void platform::FS_findGetLastTimeModified(const OSFileFind &ff, rhea::DateTime *out_dt)
{
	assert(ff.h != INVALID_HANDLE_VALUE);

	SYSTEMTIME  stime, ltime;
	FileTimeToSystemTime(&ff.findData.ftLastWriteTime, &stime);
	SystemTimeToTzSpecificLocalTime(NULL, &stime, &ltime);

	out_dt->date.setYMD(ltime.wYear, ltime.wMonth, ltime.wDay);
	out_dt->time.setHMS(ltime.wHour, ltime.wMinute, ltime.wSecond, 0);
}

//*****************************************************
void platform::FS_findClose(OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	::FindClose(ff.h);
	ff.h = INVALID_HANDLE_VALUE;
}



#endif //WIN32
