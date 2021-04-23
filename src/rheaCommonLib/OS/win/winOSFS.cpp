#ifdef WIN32
#include "winOS.h"
#include <mbstring.h>
#include <string.h>
#include <shlobj.h>
#include <strsafe.h>
#include "../../rhea.h"
#include "../../rheaString.h"


using namespace rhea;

//********************************************* 
bool win32_createFolderFromUTF8Path (const u8 *utf8_path, u32 nBytesToUseForPath)
{
	wchar_t temp[512];
	if (!platform::win32::utf8_towchar(utf8_path, nBytesToUseForPath, temp, sizeof(temp)))
		return false;

	BOOL ret = ::CreateDirectory (temp, NULL);
	if (ret == 1)
		return true;
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return true;
	return false;
}

//*****************************************************
bool platform::FS_DirectoryCreate (const u8* utf8_path)
{
	if (NULL == utf8_path)
		return false;
	if (utf8_path[0] == 0x00)
		return false;

	if (utf8_path[1] != ':')
	{
		DBGBREAK;
		return false;
	}

	u32 n = 3;
	while (utf8_path[n] != 0x00)
	{
		if (utf8_path[n]=='\\' || utf8_path[n]=='/')
		{
			if (!win32_createFolderFromUTF8Path(utf8_path,n))
				return false;
		}
		n++;
	}

	return win32_createFolderFromUTF8Path(utf8_path, n);
}

//*****************************************************
bool platform::FS_DirectoryDelete(const u8* utf8_path)
{
	wchar_t temp[512];
	if (win32::utf8_towchar(utf8_path, u32MAX, temp, sizeof(temp)))
		return (::RemoveDirectory(temp) != 0); 
	return false;
}

//*****************************************************
bool platform::FS_DirectoryExists(const u8* utf8_path)
{
	wchar_t temp[512];
	if (!win32::utf8_towchar(utf8_path, u32MAX, temp, sizeof(temp)))
		return false;

	DWORD ftyp = GetFileAttributes(temp);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	return false;
}

//**************************************************************************
void platform::FS_fileClose (FILE *f)
{
    if (NULL == f)
        fclose(f);
    f = NULL;
}

//**************************************************************************
FILE* platform::FS_fileOpenForReadBinary (const u8* utf8_fullFileNameAndPath)
{
	wchar_t filename[512];
	if (!win32::utf8_towchar (utf8_fullFileNameAndPath, -1, filename, sizeof(filename)))
	{
		DBGBREAK;
		return false;
	}

	return _wfopen(filename, L"rb");
}

//**************************************************************************
FILE* platform::FS_fileOpenForWriteBinary (const u8* utf8_fullFileNameAndPath)
{
	wchar_t filename[512];
	if (!win32::utf8_towchar (utf8_fullFileNameAndPath, -1, filename, sizeof(filename)))
	{
		DBGBREAK;
		return false;
	}

	return _wfopen(filename, L"wb");
}

//**************************************************************************
FILE* platform::FS_fileOpenForReadText (const u8* utf8_fullFileNameAndPath)
{
	wchar_t filename[512];
	if (!win32::utf8_towchar (utf8_fullFileNameAndPath, -1, filename, sizeof(filename)))
	{
		DBGBREAK;
		return false;
	}

	return _wfopen(filename, L"rt");
}

//**************************************************************************
FILE* platform::FS_fileOpenForWriteText (const u8* utf8_fullFileNameAndPath)
{
	wchar_t filename[512];
	if (!win32::utf8_towchar (utf8_fullFileNameAndPath, -1, filename, sizeof(filename)))
	{
		DBGBREAK;
		return false;
	}

	return _wfopen(filename, L"wt");
}

//**************************************************************************
FILE* platform::FS_fileOpenForAppendText (const u8* utf8_fullFileNameAndPath)
{
	wchar_t filename[512];
	if (!win32::utf8_towchar (utf8_fullFileNameAndPath, -1, filename, sizeof(filename)))
	{
		DBGBREAK;
		return false;
	}

	return _wfopen(filename, L"at");
}

//*****************************************************
bool platform::FS_fileExists(const u8* utf8_filename)
{
	wchar_t temp[512];
	if (!win32::utf8_towchar(utf8_filename, u32MAX, temp, sizeof(temp)))
		return false;

	assert (sizeof(wchar_t) == sizeof(u16));
	DWORD ftyp = GetFileAttributes(temp);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;
	if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) == 0)
		return true;
	return false;
}

//*****************************************************
bool platform::FS_fileDelete(const u8* utf8_filename)
{
	wchar_t temp[512];
	if (!win32::utf8_towchar(utf8_filename, u32MAX, temp, sizeof(temp)))
		return false;
	return (::DeleteFile (temp) != 0);
}

//*****************************************************
bool platform::FS_fileRename(const u8 *utf8_path, const u8* utf8_oldFilename, const u8 *utf8_newFilename)
{
	u8 utf8_temp[512];

	wchar_t temp1[512];
	string::utf8::copyStr (utf8_temp, sizeof(utf8_temp), utf8_path);
	string::utf8::concatStr (utf8_temp, sizeof(utf8_temp), utf8_oldFilename);
	if (!win32::utf8_towchar(utf8_temp, u32MAX, temp1, sizeof(temp1)))
		return false;

	wchar_t temp2[512];
	string::utf8::copyStr (utf8_temp, sizeof(utf8_temp), utf8_path);
	string::utf8::concatStr (utf8_temp, sizeof(utf8_temp), utf8_newFilename);
	if (!win32::utf8_towchar(utf8_temp, u32MAX, temp2, sizeof(temp2)))
		return false;

	return (::MoveFileEx (temp1, temp2, 0) != 0);
}

//*****************************************************
bool platform::FS_findFirst(OSFileFind *ff, const u8* utf8_path, const u8* utf8_jolly)
{
	assert(ff->h == INVALID_HANDLE_VALUE);

	wchar_t wctemp[512];
	win32::utf8_towchar (utf8_path, u32MAX, wctemp, sizeof(wctemp));
	wcscat_s (wctemp, _countof(wctemp), L"/*.*");

	ff->h = ::FindFirstFile(wctemp, &ff->findData);
	if (ff->h == INVALID_HANDLE_VALUE)
		return false;

	strcpy_s ((char*)ff->utf8_jolly, sizeof(ff->utf8_jolly), (const char*)utf8_jolly);
	do
	{
		if ((ff->findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0) continue;
		if ((ff->findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) continue;
		if ((ff->findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0) continue;
		
		win32::wchar_to_utf8 (ff->findData.cFileName, u32MAX, ff->utf8_curFilename, sizeof(ff->utf8_curFilename));
		if (FS_findIsDirectory(*ff) || fs::doesFileNameMatchJolly(ff->utf8_curFilename, utf8_jolly))
			return true;
	} while (FS_findNext(*ff));
	
	FS_findClose(*ff);
	return false;	
}

//*****************************************************
bool platform::FS_findNext(OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	while (FindNextFile(ff.h, &ff.findData))
	{
		if ((ff.findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0) continue;
		if ((ff.findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) continue;
		if ((ff.findData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0) continue;

		win32::wchar_to_utf8 (ff.findData.cFileName, u32MAX, ff.utf8_curFilename, sizeof(ff.utf8_curFilename));
		if (FS_findIsDirectory(ff) || fs::doesFileNameMatchJolly(ff.utf8_curFilename, ff.utf8_jolly))
			return true;
	}
	return false;
}

//*****************************************************
bool platform::FS_findIsDirectory(const OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	return ((ff.findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
}

//*****************************************************
const u8* platform::FS_findGetFileName(const OSFileFind &ff)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	return ff.utf8_curFilename;
}

//*****************************************************
void platform::FS_findGetFileName(const OSFileFind &ff, u8 *out, u32 sizeofOut)
{
	assert(ff.h != INVALID_HANDLE_VALUE);
	strcpy_s ((char*)out, sizeofOut, (const char*)ff.utf8_curFilename);
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


//*****************************************************
bool platform::FS_findFirstHardDrive(OSDriveEnumerator *h, rheaFindHardDriveResult *out)
{
	h->logicalDrives = GetLogicalDrives();
	h->current = 0;
	return FS_findNextHardDrive(*h, out);
}

//*****************************************************
bool platform::FS_findNextHardDrive(OSDriveEnumerator &h, rheaFindHardDriveResult *out)
{
	while (h.current < 26)
	{
		if ((h.logicalDrives & (0x0001 << h.current)) == 0)
		{
			h.current++;
			continue;
		}

		out->utf8_drivePath[0] = 'A' + h.current;
		out->utf8_drivePath[1] = ':';
		out->utf8_drivePath[2] = '\\';
		out->utf8_drivePath[3] = 0x00;

		wchar_t drivePath[4];
		drivePath[0] = 'A' + h.current;
		drivePath[1] = ':';
		drivePath[2] = '\\';
		drivePath[3] = 0x00;

		wchar_t s2[256];
		wchar_t driveLabel[256];
		DWORD volumeSerialNumber, maximumComponentLength, fileSystemFlags;

		driveLabel[0] = 0;
		::GetVolumeInformation(drivePath, driveLabel, _countof(driveLabel),
			&volumeSerialNumber,
			&maximumComponentLength,
			&fileSystemFlags,
			s2, _countof(s2));
		h.current++;

		if ((fileSystemFlags & FILE_READ_ONLY_VOLUME) != 0)
			continue;
		
		win32::wchar_to_utf8 (driveLabel, u32MAX, out->utf8_driveLabel, sizeof(out->utf8_driveLabel));
		return true;
	}
	return false;
}

//*****************************************************
void platform::FS_findCloseHardDrive(OSDriveEnumerator &h)
{
	h.logicalDrives = 0;
	h.current = 0xff;
}

//********************************************* 
bool platform::FS_getDestkopPath (u8* out_path, u32 sizeof_out_path)
{
	wchar_t	tempPathToUserFolded[MAX_PATH];
	if (0 == SHGetSpecialFolderPath (NULL, tempPathToUserFolded, CSIDL_DESKTOP, FALSE))
		return false;

	size_t	n = wcslen(tempPathToUserFolded);
	for (size_t t = 0; t < n; t++)
	{
		if (tempPathToUserFolded[t] == '\\')
			tempPathToUserFolded[t] = '/';
	}

	win32::wchar_to_utf8 (tempPathToUserFolded, u32MAX, out_path, sizeof_out_path);
	fs::sanitizePathInPlace (out_path);
	return true;
}
#endif //WIN32
