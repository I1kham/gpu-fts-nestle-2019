#include "TaskImportExistingGUI.h"
#include "../rheaDB/SQLite3/SQLInterface_SQLite.h"



//*********************************************************************
void TaskImportExistingGUI::run (socketbridge::TaskStatus *status, const char *params)
{
	char templateName[128];
	char templateVer[8];
	char templateSrcPath[512];
	char userGUISrcPath[512];
	char dstPath[512];


	memset(templateName, 0, sizeof(templateName));
	memset(templateVer, 0, sizeof(templateVer));
	memset(templateSrcPath, 0, sizeof(templateSrcPath));
	memset(userGUISrcPath, 0, sizeof(userGUISrcPath));
	memset(dstPath, 0, sizeof(dstPath));

	rhea::string::parser::Iter iter;
	rhea::string::parser::Iter iter2;
	const char SEP = '§';
	iter.setup(params);

	while (1)
	{
		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateName, sizeof(templateName));

		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateVer, sizeof(templateVer));
			
		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(templateSrcPath, sizeof(templateSrcPath));
		rhea::fs::sanitizePathInPlace(templateSrcPath);

		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(userGUISrcPath, sizeof(userGUISrcPath));
		rhea::fs::sanitizePathInPlace(userGUISrcPath);


		if (!rhea::string::parser::extractValue(iter, &iter2, &SEP, 1))
			break;
		iter2.copyCurStr(dstPath, sizeof(dstPath));
		rhea::fs::sanitizePathInPlace(dstPath);


		//tutti i parametri sono validi


		//creo il folder dstPath
		if (!rhea::fs::folderCreate(dstPath))
		{
			status->setMessage("Error creating %s", dstPath);
			return;
		}

		//copio il template dentro dstPath
		status->setMessage("Copying template files...");
		if (!rhea::fs::folderCopy(templateSrcPath, dstPath))
		{
			status->setMessage("Error copying files");
			return;
		}

		//copio le opzioni utente
		char src[512];
		char dst[512];
		status->setMessage("Copying existing gui files...");

		sprintf_s(src, sizeof(src), "%s/web/upload", userGUISrcPath);
		sprintf_s(dst, sizeof(dst), "%s/web/upload", dstPath);
		if (!rhea::fs::folderCopy(src, dst))
		{
			status->setMessage("Error copying files 1");
			return;
		}

		sprintf_s(src, sizeof(src), "%s/web/config", userGUISrcPath);
		sprintf_s(dst, sizeof(dst), "%s/web/config", dstPath);
		if (!rhea::fs::folderCopy(src, dst))
		{
			status->setMessage("Error copying files 2");
			return;
		}

		//copio il DB a meno che non ci sia un disallineamento tra le versioni
		if (rhea::string::convert::toI32(templateVer) == 0)
		{
			//la versione 0 non ha alcuni campi nelle tabelle, devo aggiornare il DB
			if (!priv_nestle20_template001_importVersion0(userGUISrcPath, dstPath))
			{
				status->setMessage("Error converting old DB version");
				return;
			}
		}
		else
		{
			sprintf_s(src, sizeof(src), "%s/web/backoffice/guidb.db3", userGUISrcPath);
			sprintf_s(dst, sizeof(dst), "%s/web/backoffice/guidb.db3", dstPath);
			if (!rhea::fs::fileCopy(src, dst))
			{
				status->setMessage("Error copying files 2");
				return;
			}
		}


		//v2.0.1
		//La tabella allLanguages contiene l'elenco delle lignue supportate e che compaiono in pagina langSettings.html
		//I nomi dei linguaggi non erano corretti per cui ora li sovrascrivo con quelli definitivi
		sprintf_s(dst, sizeof(dst), "%s/web/backoffice/guidb.db3", dstPath);
		priv_updateLanguageName(dst);




		status->setMessage("OK");
		return;
	}

	//se arriviamo qui, c'e' stato un errore coi parametri
	status->setMessage("Invalid parameters");
}


//*********************************************************************
bool TaskImportExistingGUI::priv_nestle20_template001_importVersion0(const char *userGUISrcPath, const char *dstPath)
{
	rhea::SQLInterface_SQLite dbUser;
	rhea::SQLInterface_SQLite dbTemplate;

	char s[2048];
	sprintf_s (s, sizeof(s), "%s/web/backoffice/guidb.db3", userGUISrcPath);
	if (!dbUser.openDB(s))
		return false;

	sprintf_s(s, sizeof(s), "%s/web/backoffice/guidb.db3", dstPath);
	if (!dbTemplate.openDB(s))
	{
		dbUser.closeDB();
		return false;
	}
	
	bool ret = false;
	rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
	rhea::SQLRst rst;
	rst.setup(allocator, 128);
	
	dbTemplate.exec("DELETE FROM pagemenu_mmi");
	if (dbUser.q("SELECT * FROM pagemenu_mmi", &rst))
	{
		ret = true;

		for (u16 r = 0; r < rst.getRowCount(); r++)
		{
			rhea::SQLRst::Row row;
			rst.getRow(r, &row);
			
			sprintf_s(s, sizeof(s), "INSERT INTO pagemenu_mmi (UID,HIS_ID,PROGR,selNum,pageMenuImg,pageConfirmImg,optionAEnabled,optionBEnabled,allowedCupSize,linkedSelection,defaultSelectionOption,name) VALUES(%s, %s, %s, %s, '%s', '%s', %s, %s, '%s', '%s', '%s', '%s');",
				rst.getValue(row, 0), rst.getValue(row, 1), rst.getValue(row, 2), rst.getValue(row, 3),
				rst.getValue(row, 4), rst.getValue(row, 5), rst.getValue(row, 6), rst.getValue(row, 7),
				rst.getValue(row, 8), rst.getValue(row, 9), rst.getValue(row, 10), rst.getValue(row, 11));

			dbTemplate.exec(s);
		}
		
		dbTemplate.exec("DELETE FROM lang WHERE UID='MMI_NAME'");
		dbTemplate.exec("DELETE FROM lang WHERE UID='MMI_DESCR'");
		dbTemplate.exec("DELETE FROM lang WHERE UID='CUP_OPTBR'");
		dbTemplate.exec("DELETE FROM lang WHERE UID='CUP_OPTBL'");
		dbTemplate.exec("DELETE FROM lang WHERE UID='CUP_OPTAR'");
		dbTemplate.exec("DELETE FROM lang WHERE UID='CUP_OPTAL'");

		if (dbUser.q("SELECT UID,ISO,What,Message FROM lang WHERE UID='MMI_NAME' or UID='MMI_DESCR' ", &rst))
		{
			for (u16 r = 0; r < rst.getRowCount(); r++)
			{
				rhea::SQLRst::Row row;
				rst.getRow(r, &row);

				sprintf_s(s, sizeof(s), "INSERT INTO lang (UID,ISO,What,Message) VALUES('%s','%s','%s','%s');",
					rst.getValue(row, 0), rst.getValue(row, 1), rst.getValue(row, 2), rst.getValue(row, 3));
				dbTemplate.exec(s);
			}
		}

		if (dbUser.q("SELECT allowedLang,defaultLang FROM generalset WHERE HIS_ID=1", &rst))
		{
			rhea::SQLRst::Row row;
			rst.getRow(0, &row);
			sprintf_s(s, sizeof(s), "UPDATE generalset SET allowedLang='%s', defaultLang='%s' WHERE HIS_ID=1;",
				rst.getValue(row, 0), rst.getValue(row, 1));
			dbTemplate.exec(s);
		}

		if (dbUser.q("SELECT icon_numRow,icon_numIconPerRow,gotoPageStandbyAfterMSec FROM pagemenu WHERE HIS_ID=1", &rst))
		{
			rhea::SQLRst::Row row;
			rst.getRow(0, &row);
			sprintf_s(s, sizeof(s), "UPDATE pagemenu SET icon_numRow='%s', icon_numIconPerRow='%s',gotoPageStandbyAfterMSec='%s' WHERE HIS_ID=1;",
				rst.getValue(row, 0), rst.getValue(row, 1), rst.getValue(row, 2));
			dbTemplate.exec(s);
		}

		dbTemplate.exec("UPDATE history SET DataLastBuild='00000000000000' WHERE ID=1");
	}


	rst.unsetup();
	dbUser.closeDB();
	dbTemplate.closeDB();
	return ret;
}


//*********************************************************************
void TaskImportExistingGUI::priv_updateLanguageName(const char *dbPath)
{
	rhea::SQLInterface_SQLite db;

	if (!db.openDB(dbPath))
		return;

	db.exec(u8"UPDATE allLanguages SET LocalName = 'Magyar' WHERE ISO = 'HU'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Українська' WHERE ISO = 'UA'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Türkçe' WHERE ISO = 'TR'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Svenska' WHERE ISO = 'SV'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Slovenčina' WHERE ISO = 'SK'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Српски' WHERE ISO = 'SR'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Русский' WHERE ISO = 'RU'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Română' WHERE ISO = 'RO'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Português' WHERE ISO = 'PT'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Polski' WHERE ISO = 'PL'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Norsk' WHERE ISO = 'NO'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Lietuvių' WHERE ISO = 'LI'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Latviešu' WHERE ISO = 'LA'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'עברית' WHERE ISO = 'HE'");
	db.exec(u8"UPDATE allLanguages SET LocalName = '日本語' WHERE ISO = 'JP'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Suomi' WHERE ISO = 'FI'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Eesti' WHERE ISO = 'ET'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Hrvatski' WHERE ISO = 'HR'");
	db.exec(u8"UPDATE allLanguages SET LocalName = '中文' WHERE ISO = 'CN'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Čeština' WHERE ISO = 'CZ'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Български' WHERE ISO = 'BG'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Nederlands' WHERE ISO = 'NL'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Español' WHERE ISO = 'ES'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Deutsch' WHERE ISO = 'DE'");
	db.exec(u8"UPDATE allLanguages SET LocalName = 'Français' WHERE ISO = 'FR'");
	//db.exec(u8"UPDATE allLanguages SET LocalName = 'Italiano' WHERE ISO = 'IT'");
	//db.exec(u8"UPDATE allLanguages SET LocalName = 'English' WHERE ISO = 'GB'");

	db.closeDB();
}