#include "SECOOs.h"

#ifdef PLATFORM_ROCKCHIP
    #define SECOOS_SERVICES_FOLDER			"/data/seco/services"
#else
	#define SECOOS_SERVICES_FOLDER			"C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/SECOOs-services"
#endif

//************************************************************
void secoos_modemLTE_getModemDownFilename (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	rhea::string::utf8::spf (out, sizeof_out, "%s/keepMODEMdown", SECOOS_SERVICES_FOLDER);
}

/*********************************************************
* Di default il modem è attivo, se esiste il file keepMODEMDown, allora viene disattivato dall'OS
*/
bool secoos::modemLTE::isEnabled()
{
	u8 fname[128];
	secoos_modemLTE_getModemDownFilename (fname, sizeof(fname));
	if (rhea::fs::fileExists(fname))
		return false;
	return true;
}

//************************************************************
void secoos::modemLTE::enable (bool b)
{
	u8 fname[128];
	secoos_modemLTE_getModemDownFilename (fname, sizeof(fname));

	if (b)
	{
		rhea::fs::fileDelete (fname);
	}
	else
	{
		FILE *f = rhea::fs::fileOpenForWriteBinary (fname);
		u8 a = 1;
		rhea::fs::fileWrite(f, &a, 1);
		rhea::fs::fileClose(f);
	}
}

//************************************************************
void secoos::getMACAddress (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/D23_MAC", SECOOS_SERVICES_FOLDER);
	u32 nRead = rhea::fs::fileReadInPreallocatedBuffer (fname, out, sizeof_out -1);
	if (nRead)
	{
		//elimina eventuali \n \r a fine stringa
		nRead--;
		while (nRead)
		{
			if (out[nRead] == '\n' || out[nRead] == '\r' || out[nRead] == ' ')
				out[nRead--] = 0x00;
			else
				break;
		}
	}
}

//************************************************************
void secoos::getLANAddress (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/hostIP", SECOOS_SERVICES_FOLDER);
	
	u32 fsize = 0;
	u8 *buffer = rhea::fs::fileCopyInMemory (fname, rhea::getScrapAllocator(), &fsize);
	if (NULL == buffer)
		return;

	rhea::string::utf8::Iter iter;
    iter.setup (buffer, 0, fsize);

	//cerco "inet addr:"
	if (rhea::string::utf8::find (iter, "inet addr:"))
	{
		iter.advanceNChar(10);		
		
		rhea::string::utf8::Iter result;
		if (rhea::string::utf8::extractValue (iter, &result))
			result.copyStrFromCurrentPositionToEnd (out, sizeof_out);
	}

	RHEAFREE(rhea::getScrapAllocator(), buffer);
}


/************************************************************
 * Se esiste il file wifiON, allora siamo in modalità "connect to", altrimenti siamo in modalità hotspot
 */
cpubridge::eWifiMode secoos::wifi::getMode()
{
	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiON", SECOOS_SERVICES_FOLDER);
	if (rhea::fs::fileExists(fname))
		return cpubridge::eWifiMode::connectTo;
	return cpubridge::eWifiMode::hotSpot;
}

/************************************************************
* Posto che siamo in modalità hotspot, allora l'SSID dell'hotspot è composto da "Rhea_" + il MACAddress
*/
void secoos::wifi::hotspot_getSSID (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	if (cpubridge::eWifiMode::hotSpot == secoos::wifi::getMode())
	{
		u8 macAddress[32];
		secoos::getMACAddress (macAddress, sizeof(macAddress));
		rhea::string::utf8::spf (out, sizeof_out, "Rhea_%s", macAddress);
	}
}

/************************************************************
* Posto che siamo in modalità "connectTo", allora il file wifiConnected (se esiste) indica l'IP assegnato al wifi.
*/
bool secoos::wifi::connectTo_isConnected()
{
	if (secoos::wifi::getMode() != cpubridge::eWifiMode::connectTo)
		return false;

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiConnected", SECOOS_SERVICES_FOLDER);
	return rhea::fs::fileExists(fname);
}

/************************************************************
* Posto che siamo in modalità "connectTo", allora il file wifiConnected (se esiste) indica l'IP assegnato al wifi.
*/
void secoos::wifi::connectTo_getIP (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	if (secoos::wifi::getMode() == cpubridge::eWifiMode::connectTo)
	{
		u8 fname[128];
		rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiConnected", SECOOS_SERVICES_FOLDER);
		u32 nRead = rhea::fs::fileReadInPreallocatedBuffer (fname, out, sizeof_out -1);
		if (nRead)
		{
			//elimina eventuali \n \r a fine stringa
			nRead--;
			while (nRead)
			{
				if (out[nRead] == '\n' || out[nRead] == '\r' || out[nRead] == ' ')
					out[nRead--] = 0x00;
				else
					break;
			}
		}
	}
}


/************************************************************
* Posto che siamo in modalità "connectTo", allora il file wpa_key contiene SSID e pwd
*/
void secoos::wifi::connectTo_getSSID (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	if (secoos::wifi::getMode() != cpubridge::eWifiMode::connectTo)
		return;

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wpa_key", SECOOS_SERVICES_FOLDER);
	
	u32 fsize = 0;
	u8 *buffer = rhea::fs::fileCopyInMemory (fname, rhea::getScrapAllocator(), &fsize);
	if (NULL == buffer)
		return;

	rhea::string::utf8::Iter iter;
	iter.setup (buffer);

	//cerco il primo doppio apice che trovo, che corrisponde all'inizio del nome SSID
	const rhea::UTF8Char doppioApice('"');
	if (rhea::string::utf8::advanceUntil (iter, &doppioApice, 1))
	{
		rhea::string::utf8::Iter result;
		if (rhea::string::utf8::extractValue (iter, &result))
			result.copyStrFromCurrentPositionToEnd (out, sizeof_out);
	}

	RHEAFREE(rhea::getScrapAllocator(), buffer);
}

//************************************************************
void secoos::wifi::connectTo_getPwd (u8 *out, u32 sizeof_out)
{
	assert (out);
	assert (sizeof_out > 1);
	memset (out, 0, sizeof_out);

	if (secoos::wifi::getMode() != cpubridge::eWifiMode::connectTo)
		return;

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wpa_key", SECOOS_SERVICES_FOLDER);
	
	u32 fsize = 0;
	u8 *buffer = rhea::fs::fileCopyInMemory (fname, rhea::getScrapAllocator(), &fsize);
	if (NULL == buffer)
		return;

	rhea::string::utf8::Iter iter;
	iter.setup (buffer);

	//cerco il primo doppio apice che trovo, che corrisponde all'inizio del nome SSID
	const rhea::UTF8Char doppioApice('"');
	if (rhea::string::utf8::advanceUntil (iter, &doppioApice, 1))
	{
		iter.advanceOneChar();
		//avanzo fino al secondo doppio apice che corrisponde a fine SSID
		if (rhea::string::utf8::advanceUntil (iter, &doppioApice, 1))
		{
			iter.advanceOneChar();
			//cerco il terzo apice che corrisponde a inizio pwd
			if (rhea::string::utf8::advanceUntil (iter, &doppioApice, 1))
			{
				rhea::string::utf8::Iter result;
				if (rhea::string::utf8::extractValue (iter, &result))
					result.copyStrFromCurrentPositionToEnd (out, sizeof_out);
			}
		}
	}

	RHEAFREE(rhea::getScrapAllocator(), buffer);
}

//************************************************************
void secoos::wifi::setModeHotspot()
{
	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiON", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiListFull.txt", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiListShort.txt", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiConnected", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiScanON", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wpa_key", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);
}

//************************************************************
void secoos::wifi::setModeConnectTo (const u8 *ssid, const u8 *pwd)
{
	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiListFull.txt", SECOOS_SERVICES_FOLDER);
	rhea::fs::fileDelete (fname);

    rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiListShort.txt", SECOOS_SERVICES_FOLDER);
    rhea::fs::fileDelete (fname);

    rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiConnected", SECOOS_SERVICES_FOLDER);
    rhea::fs::fileDelete (fname);

	u8 a = 0x01;
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiScanON", SECOOS_SERVICES_FOLDER);
	FILE *f = rhea::fs::fileOpenForWriteBinary (fname);
	rhea::fs::fileWrite (f, &a, 1);
	rhea::fs::fileClose (f);


	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiON", SECOOS_SERVICES_FOLDER);
	f = rhea::fs::fileOpenForWriteBinary (fname);
	rhea::fs::fileWrite (f, &a, 1);
	rhea::fs::fileClose (f);

	u8 wpaKey[1204];
	rhea::string::utf8::spf (wpaKey, sizeof(wpaKey), "network={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n}\n\n", ssid, pwd);
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wpa_key", SECOOS_SERVICES_FOLDER);
	f = rhea::fs::fileOpenForWriteBinary (fname);
	rhea::fs::fileWrite (f, wpaKey, rhea::string::utf8::lengthInBytes(wpaKey));
	rhea::fs::fileClose (f);
}

//************************************************************
u8*	secoos::wifi::getListOfAvailableSSID (rhea::Allocator *allocator, u32 *out_n)
{
	assert (NULL != out_n);
	*out_n = 0;

	u8 fname[128];
	rhea::string::utf8::spf (fname, sizeof(fname), "%s/wifiListShort.txt", SECOOS_SERVICES_FOLDER);
	
	//carica il file con l'elenco delle SSID
	u32 fsize = 0;
	u8 *buffer = rhea::fs::fileCopyInMemory (fname, rhea::getScrapAllocator(), &fsize);
	if (NULL == buffer)
		return NULL;

	u8 *ret = RHEAALLOCT (u8*, allocator, fsize);
	memset (ret, 0, fsize);

	//parse del file (composto da un elenco di stringhe separate da "a capo"
	u8 s[1024];
	rhea::string::utf8::Iter iter;
	iter.setup (buffer, 0, fsize);
	while (!iter.getCurChar().isEOF())
	{
		rhea::string::utf8::Iter result;
		rhea::string::utf8::extractLine (iter, &result);
		if (result.totalLenghtInBytes() > 0)
		{
			result.copyStrFromCurrentPositionToEnd(s, sizeof(s));

			//elimina eventuali \n \r a fine stringa
			u32 len = rhea::string::utf8::lengthInBytes(s);
			len--;
			while (len)
			{
				if (s[len] == '\n' || s[len] == '\r' || s[len] == ' ' || s[len] == '\t')
					s[len--] = 0x00;
				else
					break;
			}

			if ((*out_n) > 0)
				rhea::string::utf8::concatStr (ret, fsize, "|");
			rhea::string::utf8::concatStr (ret, fsize, s);
			(*out_n)++;
		}
	}


	RHEAFREE(rhea::getScrapAllocator(), buffer);
	return ret;
}
