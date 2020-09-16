/****************************************************************
 * rheaGetURLParamOrDefault
 *
 *	cerca nella queryString il parametro [paramName] e ne ritorna il valore
 *	Se non lo trova, ritorna [defValue]
 */
function rheaGetURLParamOrDefault(paramName, defValue)
{
    var result = null;
    var items = location.search.substr(1).split("&");
    for (var i = 0; i < items.length; i++)
	{
        var tmp = items[i].split("=");
        if (tmp[0] === paramName) 
			return decodeURIComponent(tmp[1]);
    }
    return defValue;
}

/****************************************************************
 * rheaLoadScript
 *
 *	ritorna una promise
 */
function rheaLoadScript (url)
{
	return new Promise( function(resolve, reject) 
	{
		//console.log ("rheaLoadScript: " +url);
		var script = document.createElement( "script" );
		script.src = url;
		script.onload = resolve;
		script.onerror = reject;
		document.head.appendChild( script );
	});
}

/*********************************************************
 * Session (memorizza solo per sessione)
 */
function Rhea_session_clearValue (itemName)								{ sessionStorage.removeItem(itemName);}
function Rhea_session_getValue (itemName) 								{ return sessionStorage.getItem(itemName); }
function Rhea_session_getOrDefault (itemName, defaultValue)				{ var r = Rhea_session_getValue(itemName); if (r === undefined || r === null) return defaultValue; return r; }
function Rhea_session_setValue (itemName, itemValue)					{ sessionStorage.setItem(itemName, itemValue); }
function Rhea_session_clearObject (itemName)							{ Rhea_session_clearValue(itemName); }
function Rhea_session_setObject (itemName, plainJSObject)				{ Rhea_session_setValue (itemName, JSON.stringify(plainJSObject)); }
function Rhea_session_getObject (itemName) 								{ var o = Rhea_session_getValue (itemName); if (!o) return o; return JSON.parse (o); }

/*********************************************************
 * Storage (memorizza permanentemente)
 */
function Rhea_storage_clearValue (itemName)								{ localStorage.removeItem(itemName);}
function Rhea_storage_getValue (itemName) 								{ return localStorage.getItem(itemName); }
function Rhea_storage_getOrDefault (itemName, defaultValue)				{ var r = Rhea_storage_getValue(itemName); if (r === undefined || r === null) return defaultValue; return r; }
function Rhea_storage_setValue (itemName, itemValue)					{ localStorage.setItem(itemName, itemValue); }

function Rhea_getCachedSelectionName (iSel_1_48)						{ return Rhea_storage_getOrDefault ("cachedSelName" +iSel_1_48, ""); }
function Rhea_setCachedSelectionName (iSel_1_48, name)					{ return Rhea_storage_setValue ("cachedSelName" +iSel_1_48, name); }

function Rhea_getCachedSelectionPrice (iSel_1_48)						{ return Rhea_storage_getOrDefault ("cachedSelPrice" +iSel_1_48, ""); }
function Rhea_setCachedSelectionPrice (iSel_1_48, name)					{ return Rhea_storage_setValue ("cachedSelPrice" +iSel_1_48, name); }

function Rhea_getTimeStamp() 
{
	var d = new Date().toISOString();
	d = d.replace(/\./g, '');
	d = d.replace(/-/g,"");
	d = d.replace(/:/g,"");
	return d;
}


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
				console.log ("language loaded [" +twoCharISOLangCode +"]=> OK");
				Rhea_session_setValue ("lang", twoCharISOLangCode);
				resolve(1);
			})
		})
		.catch( function(result)
		{
			console.log ("language error [" +twoCharISOLangCode +"]=> " +result);
			return lang_loadLang(defaultLang);
		});
}

/******************************************************************
 * lang_getCurLangISOCode
 *
 *	Ritorna il "twoCharISOLangCode" corrente
 */
function lang_getCurLangISOCode()										{ return Rhea_session_getOrDefault("lang", defaultLang); }

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

/*********************************************************
 * MMI_getCount
 *
 *	ritorna il numero di icone da visualizzare nel main menu
 */
function MMI_getCount ()														{ return rheaMainMenuIcons.length; }

/*********************************************************
 * MMI_getDisplayName
 *
 *	ritorna la stringa da utilizzare per visualizzare il nome dell'icona.
 *	La stringa è già tradotta nel linguaggio corrente
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getDisplayName (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return rheaSelName[iIcon];
}

/*********************************************************
 * MMI_getImgForPageMenu
 *
 *	ritorna il nome della img da usare in pagina menu
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getImgForPageMenu (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageMenuImg;
}


/*********************************************************
 * MMI_getImgForPageConfirm
 *
 *	ritorna il nome della img da usare in pagina confirm
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
function MMI_getImgForPageConfirm (iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= MMI_getCount()) iIcon = MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageConfirmImg;
}

/****************************************************************
 * rheaGetElemByID
 *
 */
function rheaGetElemByID (elemID)
{
	var ret =  document.getElementById(elemID);
	if (null === ret) return undefined;
	if (undefined === ret) return undefined;
	return ret;
}

/****************************************************************
 * rheaDoesElemExistsByID
 *
 *	ritorna undefined se non trova elemID,altrimenti ritorna lo 
 *  stesso risultato di rheaGetElemByID() 
 */
function rheaDoesElemExistsByID (elemID)
{
	var ret =  document.getElementById(elemID);
	if (null === ret) return undefined;
	if (undefined === ret) return undefined;
	return ret;
}

/****************************************************************
 * rheaSetDivHTMLByName
 *
 */
function rheaSetDivHTMLByName (divID, html)
{
	rheaGetElemByID(divID).innerHTML = html;
}

/****************************************************************
 * rheaGetElemWidth
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 */
function _rheaGetElemProp (p)			{ if (p===undefined) return 0; if (p=="") return 0; return parseInt(p); }
function rheaGetElemWidth (elem)		{ return _rheaGetElemProp(elem.offsetWidth); }
function rheaGetElemHeight (elem)		{ return _rheaGetElemProp(elem.offsetHeight); }
function rheaGetElemLeft (elem)			{ return _rheaGetElemProp(elem.style.left); }
function rheaGetElemTop (elem)			{ return _rheaGetElemProp(elem.style.top); }
function rheaGetElemHTML (elem)			{ return elem.innerHTML; }
function rheaGetComboSelectedOptionValue(elem)		{ return elem.options[elem.selectedIndex].value; }

function rheaSetElemHTML (elem, html)	{ elem.innerHTML = html; }
function rheaSetElemWidth (elem, pixel)	{ elem.style.width= pixel +"px"; }
function rheaSetElemHeight (elem, pixel){ elem.style.height= pixel +"px"; }
function rheaSetElemLeft (elem, pixel)	{ elem.style.left= pixel +"px"; }
function rheaSetElemTop (elem, pixel)	{ elem.style.top= pixel +"px"; }
function rheaSetDisplayMode (elem, mode) { elem.style.display=mode;}

function rheaSetElemHREF (elem, url)	{ elem.setAttribute("href",url); }

function rheaSetElemBackgroundImage (elem, url) { elem.style.backgroundImage="url(" +url +")"; }
function rheaSetElemBackgroundColor (elem, strColor) { elem.style.backgroundColor=strColor; }
function rheaSetElemTextColor (elem, strColor) { elem.style.color=strColor; }

function rheaAddClassToElem (elem, cssClass)	{ elem.classList.add(cssClass); }
function rheaRemoveClassToElem (elem, cssClass)	{ elem.classList.remove(cssClass); }

function rheaHideElem (elem)	{ rheaSetDisplayMode(elem, "none");}
function rheaShowElem (elem)	{ rheaSetDisplayMode(elem, "block");}

function rheaSelectComboOptionByValue (elem, value)
{
	for(var i=0; i < elem.options.length; i++)
	{
		if (elem.options[i].value === value)
		{
			elem.selectedIndex = i;
			return;
		}
	}
}

/*********************************************
 * rheaREST()
 * Effettua una chiamata ajax alle api REST
 */
function rheaREST(api, callback)
{
	//if (api!="getCPULCDMsg.php" && api!="get12LEDStatus.php")
		//console.log ("rheaREST [" +api +"]");
	var url = "http://192.168.10.1/rhea/REST/" +api;
    var xmlhttp;
    // compatible with IE7+, Firefox, Chrome, Opera, Safari
    xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()
    {
        if (xmlhttp.readyState == 4)
        {
			if (xmlhttp.status == 200)
			{
				//console.log ("rheaREST RESP[" +xmlhttp.responseText +"][" +api +"]");
				callback(xmlhttp.responseText);
			}
			else
			{
				//console.log ("ERR");
				//console.log (xmlhttp);
			}
			
		}
    }
    xmlhttp.open("GET", url, true);
    xmlhttp.send();
}


/*********************************************
 * rheaLEDBtn_isOn (iLed)
 * [iLed] compreso tra 0 e 11 inclusi
 * Ritorna lo stato del LED del bottone relativo
 */
function rheaLEDBtn_isOn (iLed)
{
	if (iLed <0 || iLed>11)
		return 0;
	var ledStatus12 = Rhea_session_getOrDefault ("ledStatus12", "111111111111");
	if (ledStatus12.substr(iLed,1)=="1")
		return 1;
	return 0;
}

function rheaLEDBtn_askStatus(callback)
{
	rheaREST("get12LEDStatus.php", function(data) {
		Rhea_session_setValue ("ledStatus12", data);
		callback();
	});	
}	

