#ifdef LINUX
#include <sys/stat.h>
#include "linuxOS.h"
#include "../../rhea.h"
#include "../../rheaString.h"


using namespace rhea;

//*********************************************
bool linux_createFolderFromUTF8Path (const u8 *utf8_path, u32 nBytesToUseForPath)
{
    char path[512];
    memcpy (path, utf8_path, nBytesToUseForPath);
    path[nBytesToUseForPath] = 0x00;

    if (0 == mkdir(path, 0777))
        return true;

    if (errno == EEXIST)
        return true;
    return false;
}

//*****************************************************
bool platform::FS_DirectoryCreate (const u8 *utf8_path)
{
    if (NULL == utf8_path)
        return false;
    if (utf8_path[0] == 0x00)
        return false;


    u32 n = 1;
    while (utf8_path[n] != 0x00)
    {
        if (utf8_path[n]=='\\' || utf8_path[n]=='/')
        {
            if (!linux_createFolderFromUTF8Path(utf8_path,n))
                return false;
        }
        n++;
    }

    return linux_createFolderFromUTF8Path(utf8_path, n);
}

//*****************************************************
bool platform::FS_DirectoryDelete (const u8 *path)
{
    return (rmdir((const char*)path) == 0);
}

//*****************************************************
bool platform::FS_DirectoryExists(const u8 *path)
{
    struct stat sb;
    if (stat((const char*)path, &sb) == 0 && S_ISDIR(sb.st_mode))
        return true;
    return false;
}

//*****************************************************
bool platform::FS_fileExists(const u8 *filename)
{
    FILE *f = fopen((const char*)filename, "r");
    if (NULL == f)
        return false;
    rhea::fs::fileClose(f);
    return true;
}

//*****************************************************
bool platform::FS_fileDelete(const u8 *filename)
{
    return (remove((const char*)filename) == 0);
}

//*****************************************************
bool platform::FS_fileRename(const u8 *utf8_path, const u8 *utf8_oldFilename, const u8 *utf8_newFilename)
{
    char temp1[512];
    sprintf_s (temp1, sizeof(temp1), "%s/%s", utf8_path, utf8_oldFilename);

    char temp2[512];
    sprintf_s (temp2, sizeof(temp2), "%s/%s", utf8_path, utf8_newFilename);

    return (rename(temp1, temp2) == 0);
}

//*****************************************************
bool platform::FS_findFirst (OSFileFind *ff, const u8 *utf8_path, const u8 *utf8_jolly)
{
    assert(ff->dirp == NULL);

    char filename[1024];
    sprintf_s(filename, sizeof(filename), "%s/%s", utf8_path, utf8_jolly);

    ff->dirp = opendir((const char*)utf8_path);
    if (NULL == ff->dirp)
        return false;

    strcpy_s (ff->strJolly, sizeof(ff->strJolly), (const char*)utf8_jolly);
    return FS_findNext(*ff);
}

//*****************************************************
bool platform::FS_findNext (OSFileFind &ff)
{
    assert(ff.dirp != NULL);

    while (1)
    {
        ff.dp = readdir (ff.dirp);
        if (NULL == ff.dp)
            return false;

        //se è una dir...
        if (ff.dp->d_type == DT_DIR)
            return true;

        if (rhea::fs::doesFileNameMatchJolly ((const u8*)ff.dp->d_name, (const u8*)ff.strJolly))
            return true;
    }
}

//*****************************************************
void platform::FS_findClose(OSFileFind &ff)
{
    assert(ff.dirp != NULL);
    closedir(ff.dirp);
    ff.dirp = NULL;
}

//*****************************************************
bool platform::FS_findIsDirectory(const OSFileFind &ff)
{
    assert(ff.dirp != NULL);
    if (ff.dp->d_type == DT_DIR)
        return true;
    return false;
}

//*****************************************************
const u8 *platform::FS_findGetFileName(const OSFileFind &ff)
{
    assert(ff.dirp != NULL);
    return (const u8*)ff.dp->d_name;
}

//*****************************************************
void platform::FS_findGetFileName (const OSFileFind &ff, u8 *out, u32 sizeofOut)
{
    assert(ff.dirp != NULL);
    sprintf_s((char*)out, sizeofOut, "%s", ff.dp->d_name);
}

//*****************************************************
void platform::FS_findGetCreationTime(const OSFileFind &ff, rhea::DateTime *out_dt UNUSED_PARAM)
{
    assert(ff.dirp != NULL);
    //TODO
    DBGBREAK;
}

//*****************************************************
void platform::FS_findGetLastTimeModified(const OSFileFind &ff, rhea::DateTime *out_dt UNUSED_PARAM)
{
    assert(ff.dirp != NULL);
    //TODO
    DBGBREAK;
}


//*****************************************************
bool platform::FS_findFirstHardDrive(OSDriveEnumerator *h UNUSED_PARAM, rheaFindHardDriveResult *out UNUSED_PARAM)
{
    //TODO
	DBGBREAK;
	return false;
}

//*****************************************************
bool platform::FS_findNextHardDrive(OSDriveEnumerator &h UNUSED_PARAM, rheaFindHardDriveResult *out UNUSED_PARAM)
{
    //TODO
	DBGBREAK;
	return false;

}

//*****************************************************
void platform::FS_findCloseHardDrive(OSDriveEnumerator &h UNUSED_PARAM)
{	
}

//*****************************************************
bool platform::FS_getDestkopPath(u8* out_path UNUSED_PARAM, u32 sizeof_out_path UNUSED_PARAM)
{
	//TODO
	DBGBREAK;
	return false;
}

//**************************************************************************
void platform::FS_fileClose (FILE *f)
{
    if (NULL != f)
        fclose(f);
    f = NULL;
}

//**************************************************************************
FILE* platform::FS_fileOpenForReadBinary (const u8 *utf8_fullFileNameAndPath)
{
    return fopen((const char*)utf8_fullFileNameAndPath, "rb");
}

//**************************************************************************
FILE* platform::FS_fileOpenForWriteBinary (const u8 *utf8_fullFileNameAndPath)
{
    return fopen((const char*)utf8_fullFileNameAndPath, "wb");
}

//**************************************************************************
FILE* platform::FS_fileOpenForReadText (const u8 *utf8_fullFileNameAndPath)
{
    return fopen((const char*)utf8_fullFileNameAndPath, "rt");
}

//**************************************************************************
FILE* platform::FS_fileOpenForWriteText (const u8 *utf8_fullFileNameAndPath)
{
    return fopen((const char*)utf8_fullFileNameAndPath, "wt");
}

//**************************************************************************
FILE* platform::FS_fileOpenForAppendText (const u8 *utf8_fullFileNameAndPath)
{
    return fopen((const char*)utf8_fullFileNameAndPath, "at");
}
#endif //LINUX
