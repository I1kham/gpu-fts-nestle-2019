#ifdef LINUX
#include <sys/stat.h>
#include "linuxOS.h"
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
            memcpy(temp, path, n);
			temp[n] = 0;

            if (!FS_DirectoryExists(temp))
            {
                if (0 != mkdir(temp, 0777))
                    return false;
            }
		}
	}

    if (0 == mkdir(path, 0777))
        return true;

    if (errno == EEXIST)
        return true;
    return true;
}

//*****************************************************
bool platform::FS_DirectoryDelete (const char *path)
{
    return (rmdir(path) == 0);
}

//*****************************************************
bool platform::FS_DirectoryExists(const char *path)
{
    struct stat sb;
    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode))
        return true;
    return false;
}

//*****************************************************
bool platform::FS_fileExists(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (NULL == f)
        return false;
    fclose(f);
    return true;
}

//*****************************************************
bool platform::FS_fileDelete(const char *filename)
{
    return (remove(filename) == 0);
}

//*****************************************************
bool platform::FS_fileRename(const u8 *utf8_path, const u8* utf8_oldFilename, const u8 *utf8_newFilename)
{
	return (rename(oldFilename, newFilename) == 0);
}


//*****************************************************
bool platform::FS_findFirst(OSFileFind *ff, utf8_path, const u8* const utf8_jolly)
{
    assert(ff->dirp == NULL);

    char filename[1024];
    sprintf_s(filename, sizeof(filename), "%s/%s", strPathNoSlash, strJolly);

    ff->dirp = opendir(strPathNoSlash);
    if (NULL == ff->dirp)
        return false;

    strcpy_s (ff->strJolly, sizeof(ff->strJolly), strJolly);
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

        if (rhea::fs::doesFileNameMatchJolly (ff.dp->d_name, ff.strJolly))
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
const char* platform::FS_findGetFileName(const OSFileFind &ff)
{
    assert(ff.dirp != NULL);
    return ff.dp->d_name;
}

//*****************************************************
void platform::FS_findGetFileName(const OSFileFind &ff, char *out, u32 sizeofOut)
{
    assert(ff.dirp != NULL);
    sprintf_s(out, sizeofOut, "%s", ff.dp->d_name);
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
bool platform::FS_getDestkopPath(u8* out_path, u32 sizeof_out_path)
{
	//TODO
	DBGBREAK;
	return false;
}
#endif //LINUX
