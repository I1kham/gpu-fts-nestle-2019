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
bool platform::FS_fileRename(const char *oldFilename, const char *newFilename)
{
	return (rename(oldFilename, newFilename) == 0);
}


//*****************************************************
bool platform::FS_findFirst(OSFileFind *ff, const char *strPathNoSlash, const char *strJolly)
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

        //se � una dir...
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

    DBGBREAK;
}

//*****************************************************
void platform::FS_findGetLastTimeModified(const OSFileFind &ff, rhea::DateTime *out_dt UNUSED_PARAM)
{
    assert(ff.dirp != NULL);

    DBGBREAK;
}





#endif //LINUX
