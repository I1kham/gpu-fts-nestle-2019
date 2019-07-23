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
Rhea.prototype.Session_clearValue = function (itemName)
{
	store.remove(itemName);
}
/*********************************************************
 * Session_getValue
 *
 */
Rhea.prototype.Session_getValue = function (itemName)
{
	//return localStorage.getItem (itemName);
	return store.get(itemName);
}

Rhea.prototype.Session_getOrDefault = function (itemName, defaultValue)
{
	var r = this.Session_getValue(itemName);
	if (r === undefined)
		return defaultValue;
	return r;
}
/*********************************************************
 * Session_setValue
 *
 */
Rhea.prototype.Session_setValue = function (itemName, itemValue)
{
	//localStorage.setItem (itemName, itemValue);
	store.set(itemName, itemValue);
}

/*********************************************************
 * Session_clearObject
 *
 */
Rhea.prototype.Session_clearObject = function (itemName, plainJSObject)
{
	this.Session_clearValue (itemName);
}


/*********************************************************
 * Session_setObject
 *
 * trasforma [plainJSObject] nella sua rappresentazione JSON e lo memorizza
 * Lavora in coppia con this.Session_getObject 
 */
Rhea.prototype.Session_setObject = function (itemName, plainJSObject)
{
	this.Session_setValue (itemName, JSON.stringify(plainJSObject));
}

/*********************************************************
 * Session_getObject
 *
 * riturna un plain JS object a partire dalla sua descrizione in formato JSON.
 * Lavora in coppia con this.Session_setObject
 */
Rhea.prototype.Session_getObject = function (itemName)
{
	var o = this.Session_getValue (itemName);
	if (!o)
		return o;
	return JSON.parse (o);
}

