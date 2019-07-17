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
Rhea.prototype.lang_loadLang = function (twoCharISOLangCode)
{
	var me = this;
	
	var url = "config/lang/" +twoCharISOLangCode +"/translation.js";

	//carica lo script js che contiene le traduzioni principali nella lingua selezionata
	return rheaLoadScript(url)
		.then( function(result) 
		{
			return new Promise( function(resolve, reject) 
			{
				rheaLog ("language loaded [" +twoCharISOLangCode +"]=> OK");
				me.Session_setValue ("lang", twoCharISOLangCode);
				resolve(1);
			})
		})
		.catch( function(result)
		{
			rheaLog ("language error [" +twoCharISOLangCode +"]=> " +result);
			return me.lang_loadLang(RHEA_DEFAULT_FALLOFF_LANGUAGE);
		});
}

/******************************************************************
 * lang_getCurLangISOCode
 *
 *	Ritorna il "twoCharISOLangCode" corrente
 */
Rhea.prototype.lang_getCurLangISOCode = function ()
{
	return this.Session_getValue ("lang");
}

/******************************************************************
 * lang_loadContent
 *
 *	carica lo script [nomeFileJS].js dalla cartella associata al linguaggio attualmente in uso.
 *	Ritorna una promise
 */
Rhea.prototype.lang_loadJS = function (nomeFileJS)
{
	var langISO = this.Session_getValue ("lang");
	var url = "config/lang/" +langISO +"/" +nomeFileJS +".js";
	return rheaLoadScript(url);
}
