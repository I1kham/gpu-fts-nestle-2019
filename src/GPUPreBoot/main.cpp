#include "main.h"
#include <stdarg.h>

//*****************************************************
void rpf (u8 *dest, u32 sizeOfDest, const char *format, ...)
{
    va_list argptr;
    va_start(argptr, format);
    vsnprintf (reinterpret_cast<char*>(dest), sizeOfDest, format, argptr);
    va_end(argptr);
}

//*****************************************************
bool usbPenDriveExists()
{
    u8 path[256];
    rpf (path, sizeof(path), "%s/rhea/GPU210", USB_MOUNTPOINT);
    return rhea::fs::folderExists (path);
}

/*****************************************************
 * Cerca sulla USB nella cartella rhea/GPU210 eventuali file con estensione mh211
 * Se ne trova una, procede con l'aggiornamento del FW
 */
bool checkFWUpdate()
{
    //verifico la presenza di file con estensione mh211 che sono dei
    //tar con dentro la nuova versione del GPUBoot
    OSFileFind ff;
    u8 path[256];

    rpf (path, sizeof(path), "%s/rhea/GPU210", USB_MOUNTPOINT);
    if (!rhea::fs::findFirst (&ff, path, (const u8*)"*.mh211"))
        return false;

    do
    {
        if (rhea::fs::findIsDirectory(ff))
            continue;

        //ho trovato un file con estensione mh211, lo copio nella cartella attuale e lo scompatto
        const u8 *fname = rhea::fs::findGetFileName(ff);
        printf ("found %s\n", fname);
        u8 s1[512];
        u8 s2[512];
        rpf (s1, sizeof(s1), "%s/%s", path, fname);
        rpf (s2, sizeof(s2), "%s/gpuBootUpdate", rhea::getPhysicalPathToAppFolder());
        if (rhea::fs::fileCopy (s1, s2))
        {
            //untar -xvf gpuBootUpdate
            chdir((const char*)rhea::getPhysicalPathToAppFolder());
            rpf (s1, sizeof(s1), "/bin/tar -xvf %s/gpuBootUpdate", rhea::getPhysicalPathToAppFolder());
            system((const char*)s1);
        }
        rhea::fs::findClose(ff);
        return true;

    } while (rhea::fs::findNext (ff));

    rhea::fs::findClose(ff);
    return false;
}


//*****************************************************
int main(int argc, char *argv[])
{
    //sposto la "working directory" nel path di questo exe
    if (argc > 0)
    {
        if (argv[0][0] != '.')
        {
            //recupero il path direttamente da argv[0]
            u8 path[256];
            rhea::fs::extractFilePathWithOutSlash ((const u8*)argv[0], path, sizeof(path));
            chdir((const char*)path);
        }
    }

#ifdef WIN32
    HINSTANCE hInst = NULL;
    rhea::init("rheaGPUPreBoot", &hInst);
#else
    rhea::init("rheaGPUPreBoot", NULL);
#endif


    if (usbPenDriveExists())
        checkFWUpdate();

    //esegue ROCKCHIP_RELEASE_GPUBoot
    char exeAndPathName[512];
    sprintf_s (exeAndPathName, sizeof(exeAndPathName), "%s/ROCKCHIP_RELEASE_GPUBoot", rhea::getPhysicalPathToAppFolder());

    rhea::deinit();

    {
        const char *argv[4];
        memset (argv, 0, sizeof(argv));
        argv[0] = exeAndPathName;
        printf ("launcing %s\n", exeAndPathName);
        execvp (exeAndPathName, (char* const*)argv);
    }
    return 0;
}
