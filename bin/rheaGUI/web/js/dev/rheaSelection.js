/*********************************************************
 * selection_createEmpty
 *
 * E' un plain-object con le seguenti prop:
 *
 * 	selNum: 		da 1 a N, indica il numero di selezione cosi' come definito nel da2
 *	enabled:		[0|1]	indica se la selezione è "fattibile", ovvero se non ci sono OFF che ne impediscono l'erogazione
 *	price:			stringa formattata con i decimali e il separatore decimale al punto giusto
 */
Rhea.prototype.selection_createEmpty = function(selNumber)
{
	var s = {
		selNum: 		selNumber,
		enabled: 		0,
		price: 			"0.00"
	};
	return s;
}


/*********************************************************
 * selection_getCount
 *
 *	ritorna il numero di selezioni massime utilizzabili
 */
Rhea.prototype.selection_getCount = function()
{
	return RHEA_NUM_MAX_SELECTIONS;
}

/*********************************************************
 * selection_getBySelNumber
 *
 *	[selNum]	=> da 1 a RHEA_NUM_MAX_SELECTIONS
 */
Rhea.prototype.selection_getBySelNumber = function (selNum)
{
	if (selNum < 1 || selNum > RHEA_NUM_MAX_SELECTIONS)
	{
		rheaLog ("ERR:Rhea.selection_getBySelNumber(" +selNum +") => invalid sel number");
		return this.selection_createEmpty(0);
	}
	
	return this.selectionList[selNum-1];
}

/*********************************************************
 * selection_sessionRestore
 *
 *	leggo tutto dal "db"
 */
Rhea.prototype.selection_sessionRestore = function()
{
	rheaLog ("Rhea.selection_sessionRestore()");
	this.selectionList = [];
	for (var i=1; i<=RHEA_NUM_MAX_SELECTIONS; i++)
	{
		var s = Rhea_session_getObject("selInfo" +i);
		if (s === undefined)
		{
			s = this.selection_createEmpty(i);
			Rhea_session_setObject("selInfo" +i, s);
		}
		this.selectionList.push (s);
	}
}

/*********************************************************
 * selection_sessionStore
 *
 *	salva tutto nel "db"
 */
Rhea.prototype.selection_sessionStore = function()
{
	rheaLog ("Rhea.selection_sessionStore()");
	for (var i=1; i<=RHEA_NUM_MAX_SELECTIONS; i++)
	{
		Rhea_session_setObject("selInfo" +i, this.selectionList[i-1]);
	}
}

/*********************************************************
 * selection_start
 *
 *	[iSelNumber] >0 <= [selection_getCount()]
 *	[iSelNumber] deve essere un intero, non un char
 *
 *	Chiede alla GPU di iniziare una selezione.
 *	Per monitorare lo stato della richiesta, utilizzare l'evento rhea.onEvent_selectionReqStatus()
 */
Rhea.prototype.selection_start = function(iSelNumber)
{
	var buffer = new Uint8Array(2);
	buffer[0] = RHEA_EVENT_START_SELECTION;
	buffer[1] = parseInt(iSelNumber);
	this.sendGPUCommand ("E", buffer, 0, 0);
}

/*********************************************************
 * selection_stop
 *
 *	Chiede alla GPU di fermare la selezione corrente
 */
Rhea.prototype.selection_stop = function()
{
	var buffer = new Uint8Array(1);
	buffer[0] = RHEA_EVENT_STOP_SELECTION;
	this.sendGPUCommand ("E", buffer, 0, 0);
}