/*********************************************************
 * rheaSession
 *
 * Fornisce dei metodi per la memorizzazione/recupero di valori che rimangono
 * persistenti anche cambiando pagina (da una url all'altra).
 * 
 *	Session_setValue(itemName, itemValue)	=>	setta il valore di itemName
 *	Session_getValue(itemName)				=>	ritorna "undefined" oppure il valore di "itemName"
 *
 *	Session_setObject(itemName, plainJSObject)	=>	memorizza un plain object in formato json
 *	Session_getObject(itemName)					=>	ritorna "undefined" oppure il plain object 
 *
 */

/*********************************************************
 * Session_clearValue
 *
 * elimina un entry dal db
 */
function Rhea_session_clearValue (itemName)							{ store.remove(itemName);}

/*********************************************************
 * Session_getValue
 *
 */
function Rhea_session_getValue (itemName) 							{ return store.get(itemName); }
function Rhea_session_getOrDefault (itemName, defaultValue)
{
	var r = Rhea_session_getValue(itemName);
	if (r === undefined)
		return defaultValue;
	return r;
}

/*********************************************************
 * Session_setValue
 *
 */
function Rhea_session_setValue (itemName, itemValue)					{ store.set(itemName, itemValue); }

/*********************************************************
 * Session_clearObject
 *
 */
function Rhea_session_clearObject (itemName)								{ Rhea_session_clearValue(itemName); }


/*********************************************************
 * Session_setObject
 *
 * trasforma [plainJSObject] nella sua rappresentazione JSON e lo memorizza
 */
function Rhea_session_setObject (itemName, plainJSObject)					{ Rhea_session_setValue (itemName, JSON.stringify(plainJSObject)); }


/*********************************************************
 * Session_getObject
 *
 * riturna un plain JS object a partire dalla sua descrizione in formato JSON.
 * Lavora in coppia con Rhea_session_setObject
 */
function Rhea_session_getObject (itemName) 
{
	var o = Rhea_session_getValue (itemName);
	if (!o)
		return o;
	return JSON.parse (o);
}

