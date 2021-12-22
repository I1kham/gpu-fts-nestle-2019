/**************************************************************************************************************
 * GPUUpdater risiede nella cartella principale insieme allo script startRhea.sh e ha una sottocartella di nome GPUPackage2019 dentro
 * la quale c'è l'eseguibile della GPU
 *
 * startRhea.sh è lo script che viene eseguito all'accensione della macchina.
 * startRhea.sh fa partire GPUUpdater il quale aggiorna eventualmente la GPU unzippando il file .. che trova in GPUPackage2019/autoUpdate
 * e poi fa partire la GPU invocando GPUPackage2019/GPUFusion
 *
 * Lo scopo di GPUUpdater quindi è quello di consentire di aggiornare la GPU senza passare dal boot storico di rhea del quale non abbiamo i sorgenti
 */
#include "../rheaCommonLib/rhea.h"


//*****************************************************
bool executeShellCommandAndStoreResult (const char *shellCommand, char *out_result, u32 sizeOfOutResult)
{
    FILE *fp = popen (shellCommand, "r");
    if (NULL == fp)
        return false;
    fgets (out_result, sizeOfOutResult, fp);
    fclose (fp);
    return true;
}

//*****************************************************
void checkForGPUUpdate()
{
#ifdef _DEBUG
    printf ("checkForGPUUpdate\n");
#endif
    u8 s1[512];
    rhea::string::utf8::spf (s1, sizeof(s1), "%s/GPUPackage2019/autoUpdate", rhea::getPhysicalPathToAppFolder());
    if (!rhea::fs::folderExists(s1))
    {
#ifdef _DEBUG
        printf ("%s not found\n", s1);
#endif
        return;
    }

    u8 s2[512];
    if (!rhea::fs::findFirstFileInFolderWithJolly (s1, (const u8*)"*.mh6", true, s2, sizeof(s2)))
    {
#ifdef _DEBUG
        printf ("no GPU update found in %s\n", s1);
#endif
        return;
    }

    //copio l'aggiornamento spostandolo dalla cartella GPUPackage2019/autoUpdate e mettendolo in /
    rhea::string::utf8::spf (s1, sizeof(s1), "%s/update.mh6", rhea::getPhysicalPathToAppFolder());
    rhea::fs::fileCopy (s2, s1);
#ifdef _DEBUG
    printf ("copying %s into %s\n", s2, s1);
#endif

    //elimino il file di update dalla cartella GPUPackage2019/autoUpdate
#ifdef _DEBUG
    printf ("deleting %s\n", s2);
#endif
    rhea::fs::fileDelete (s2);


    //unzippo
    rhea::string::utf8::spf (s2, sizeof(s2), "tar -xf %s", s1);
#ifdef _DEBUG
    printf ("%s\n", s2);
#endif
    char result[32];
    memset (result, 0, sizeof(result));
    executeShellCommandAndStoreResult ((const char*)s2, result, sizeof(result));
}

//*****************************************************
void launchGPU()
{
    u8 s[512];
    rhea::string::utf8::spf (s, sizeof(s), "%s/GPUPackage2019", rhea::getPhysicalPathToAppFolder());
    chdir((const char*)s);

    rhea::string::utf8::spf (s, sizeof(s), "%s/GPUPackage2019/GPUFusion", rhea::getPhysicalPathToAppFolder());
#ifdef _DEBUG
    printf ("launching GPU %s\n", s);
#endif
    rhea::shell_runCommandNoWait ((const char*)s);
}

//*****************************************************
int main()
{
    rhea::init("GPUUpdt", NULL);

    checkForGPUUpdate();
    launchGPU();
	
    rhea::deinit();
	return 0;
}


