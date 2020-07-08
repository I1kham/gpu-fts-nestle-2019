/******************************************************************
 * lang_loadLang
 *
 *	Ritorna una promise che viene soddisfatta quando il nuovo linguaggio è stato caricato.
 *
 *	Il parametro [twoCharISOLangCode] deve essere un stringa di 2 caratteri rappresentativa del linguaggio da caricare.
 *	Le traduzioni si trovano nella file config/lang/twoCharISOLangCode/translation.js
 *
 *	Se si richiede un linguaggio non disponibile (ie: il file config/lang/twoCharISOLangCode/translation.js non esiste), allora viene automaticamente
 *	caricato il linguaggio RHEA_DEFAULT_FALLOFF_LANGUAGE, che si suppone esistere sempre
 */
function lang_loadLang (twoCharISOLangCode)
{
	var url = "config/lang/" +twoCharISOLangCode +".js";

	//carica lo script js che contiene le traduzioni principali nella lingua selezionata
	return rheaLoadScript(url)
		.then( function(result) 
		{
			return new Promise( function(resolve, reject) 
			{
				rheaLog ("language loaded [" +twoCharISOLangCode +"]=> OK");
				Rhea_session_setValue ("lang", twoCharISOLangCode);
				resolve(1);
			})
		})
		.catch( function(result)
		{
			rheaLog ("language error [" +twoCharISOLangCode +"]=> " +result);
			return lang_loadLang(RHEA_DEFAULT_FALLOFF_LANGUAGE);
		});
}

/******************************************************************
 * lang_getCurLangISOCode
 *
 *	Ritorna il "twoCharISOLangCode" corrente
 */
function lang_getCurLangISOCode()												{ return Rhea_session_getValue("lang"); }

/******************************************************************
 * lang_loadContent
 *
 *	carica lo script [nomeFileJS].js dalla cartella associata al linguaggio attualmente in uso.
 *	Ritorna una promise
 */
function lang_loadJS (nomeFileJS)
{
	var langISO = lang_getCurLangISOCode();
	var url = "config/lang/" +langISO +"_" +nomeFileJS +".js";
	return rheaLoadScript(url);
}
