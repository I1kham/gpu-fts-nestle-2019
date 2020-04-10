#include "varie.h"


/*****************************************************
 * La tabella allLanguages contiene l'elenco delle lignue supportate e che compaiono in pagina langSettings.html
 * I nomi dei linguaggi non erano corretti per cui ora li sovrascrivo con quelli definitivi.
 *
 * Questa è una fn di comodo eseguita "una tantum" per andare a modificare tutti i DB dei template senza doverli passare a mano uno ad uno 
 */
void varie_patchAllTemplate_updateLanguageNames()
{
	char s[1024];
	sprintf_s(s, sizeof(s), "%s/template/nestle-2.0-template-001/prefabs", rhea::getPhysicalPathToAppFolder());
	
	OSFileFind ff;
	if (platform::FS_findFirst(&ff, s, "*.*"))
	{
		do
		{
			if (!platform::FS_findIsDirectory(ff))
				continue;
			const char *folderName = platform::FS_findGetFileName(ff);
			if (folderName[0] == '.')
				continue;

			sprintf_s(s, sizeof(s), "%s/template/nestle-2.0-template-001/prefabs/%s/backoffice/guidb.db3", rhea::getPhysicalPathToAppFolder(),folderName);
			TaskImportExistingGUI::priv_updateLanguageName(s);

		} while (platform::FS_findNext(ff));
		platform::FS_findClose(ff);
	}
}

