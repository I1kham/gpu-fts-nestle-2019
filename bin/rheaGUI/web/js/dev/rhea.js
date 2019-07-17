//Se si richiede un linguaggio che non è supportato (ie: mancano i file), allora si carica il RHEA_DEFAULT_FALLOFF_LANGUAGE
var 	RHEA_DEFAULT_FALLOFF_LANGUAGE = "GB";

//costanti di comodo da passare come parametro alla fn requestGPUEvent() e/o ricevuti come eventi dalla GPU
var 	RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED = 97;
var	RHEA_EVENT_SELECTION_PRICES_UPDATED = 98;
var	RHEA_EVENT_CREDIT_UPDATED = 99;
var	RHEA_EVENT_CPU_MESSAGE = 100;
var	RHEA_EVENT_SELECTION_REQ_STATUS = 101;
var	RHEA_EVENT_START_SELECTION = 102;
var	RHEA_EVENT_STOP_SELECTION = 103;


/*********************************************************

 * Rhea
 *
 * costruttore
 */
function Rhea()
{
	//lingua corrente
	if (this.Session_getValue ("lang") === undefined)
		this.Session_setValue ("lang", RHEA_DEFAULT_FALLOFF_LANGUAGE);
	
	//ajax    
	this.nextAjaxRequestID = 1;
	this.ajaxReceivedAnswerQ = [];
	var NUM_MAX_CONCURRENT_AJAX_REQUEST = 16;	//num max di richieste "ajax" contemporanee
	for (var i=0; i<NUM_MAX_CONCURRENT_AJAX_REQUEST; i++)
		this.ajaxReceivedAnswerQ[i] = null;
	

	//Selezioni: array di sessione.
	//Alcune informazioni sullo stato delle selezioni sono permanenti, nel senso che sono memorizzate in "session".
	//L'aggiornamento di queste informazioni viene gestito in autonomia e le pagine possono sempre in ogni momento richiedere lo stato delle selezioni
	//utilizzando la fn selection_getBySelNumber()
	//Per sapere quali informazioni sono disponibili per ogni selezione, cerca la fn selection_createEmpty()
	this.nMaxSelection = 48;
	this.selection_sessionRestore();
	
	
	//Variabile di sessione: credito attuale
	//Viene gestita in autonomia, in modo che this.credit rifletta sempre l'attuale credito disponibile
	if (!this.Session_getValue("credit"))
	{
		this.credit = "0";
		this.Session_setValue("credit", "0");
	}
	else
		this.credit = this.Session_getValue("credit");
}

/******************************************************
 * clearSessionData
 * 
 * Utile per pulire le informazioni di sessione. Di solito usata allo startup della GUI
 */
Rhea.prototype.clearSessionData = function ()
{
	this.selection_sessionClear();
	this.Session_setValue ("lang", RHEA_DEFAULT_FALLOFF_LANGUAGE);
	this.credit = "0";
	this.Session_setValue("credit", "0");
}



/******************************************************
 * webSocket_connect
 *
 * 	Ritorna una promise che viene risolta quando la connessione è stabilita.
 *	In caso di errori, viene fatto automaticamente un retry della connessione. 
 *	Continua a fare retry fino a che la connessione è stabilita.
 */
Rhea.prototype.webSocket_connect = function()
{
	var me = this;
	return new Promise( function(resolve, reject) 
	{
		var tryConnect = function()
		{
			rheaLog("Rhea:: trying to connect...");
			me.websocket = new WebSocket("ws://127.0.0.1:2280/", "binary");
			me.websocket.onopen = 		function(evt) { rheaLog("Rhea::webSocket connected"); resolve(1); };
			me.websocket.onclose = 		function(evt) { me.webSocket_onClose(evt); };
			me.websocket.onmessage = 	function(evt) { me.webSocket_onRcv(evt) };
			me.websocket.onerror = 		function(evt) { rheaLog("Rhea::onWebSocketErr => ERROR: " + evt.data); setTimeout(tryConnect, 2000); };
		}
		
		setTimeout(tryConnect, 1);
	});
}



/*********************************************************
 * Websocket event handling
 *
 */
Rhea.prototype.webSocket_onClose = function (evt)
{
	rheaLog("Rhea::webSocket_onClose =>  Disconnected");
	this.websocket.close();
}

Rhea.prototype.webSocket_onRcv = function (evt)
{
	//rheaLog("RESPONSE: [len=" +evt.data.size +"] " + evt.data);
	var me = this;
	
	var fileReader = new FileReader();
	fileReader.readAsArrayBuffer(evt.data);
	fileReader.onload = function(event) 
	{
		var data = new Uint8Array(fileReader.result);
		
		//rheaLog ("Rhea::RCV [len=" +data.length +"] [" +utf8ArrayToStr(data) +"]");
		
		//vediamo se è una risposta "ajax" (il buffer deve iniziare con #AJ.ja dove al posto di "." c'è la requestID
		if (data.length > 6)
		{
			if (data[0] == 35 && data[1] == 65 && data[2] == 74 && data[4] == 106 && data[5] == 97)
			{
				var reqID = parseInt(data[3]);
	
				for (var i=0; i<me.ajaxReceivedAnswerQ.length; i++)
				{
					if (me.ajaxReceivedAnswerQ[i] == null)
						continue;
					if (me.ajaxReceivedAnswerQ[i].requestID == reqID)
					{
						me.ajaxReceivedAnswerQ[i].rcv = utf8ArrayToStr(data.subarray(6));
						return;
					}
				}
				return;
			}
		}
		
		//vediamo se è un evento inviato dalla GPU (il buffer deve iniziare con #eVn. dove al posto di "." c'è un byte che indica il tipo di evento
		if (data.length >= 5)
		{
			if (data[0] == 35 && data[1] == 101 && data[2] == 86 && data[3] == 110)
			{
				var eventTypeID = parseInt(data[4]);
				var eventSeqNum = parseInt(data[5]);
				var payloadLen = parseInt(256 * data[6]) + parseInt(data[7]);
				switch (eventTypeID)
				{
				case RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED:
					{
						rheaLog ("RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED:");
						var nSel = parseInt(data[8]);
						for (var i=1; i<=nSel; i++)
							me.selection_getBySelNumber(i).enabled = parseInt(data[8+i]);
						me.selection_sessionStore();
						me.onEvent_selectionAvailabilityUpdated();
					}
					break;
					
				case RHEA_EVENT_SELECTION_PRICES_UPDATED:
					{
						rheaLog ("RHEA_EVENT_SELECTION_PRICES_UPDATED:");
						var prices = utf8ArrayToStr(data.subarray(8)).split("§")
						for (var i=1; i<=prices.length; i++)
							me.selection_getBySelNumber(i).price = prices[i-1];
						me.selection_sessionStore();
						me.onEvent_selectionPricesUpdated();
					}
					break;
					
				case RHEA_EVENT_CREDIT_UPDATED:
					{
						rheaLog ("RHEA_EVENT_CREDIT_UPDATED:");
						me.credit = utf8ArrayToStr(data.subarray(8));
						me.Session_setValue("credit", me.credit);
						me.onEvent_creditUpdated();
					}
					break;
					
				case RHEA_EVENT_CPU_MESSAGE:
					{
						var importanceLevel = data[8];
						var lenInBytes = data[9] * 256 + data[10];
						var msg = utf8ArrayToStr(data.subarray(11));
						rheaLog ("RHEA_EVENT_CPU_MESSAGE: [" +importanceLevel +"] " +msg);
						if (msg != "")
							me.onEvent_cpuMessage(msg, importanceLevel);
					}
					break;					
				
				case RHEA_EVENT_SELECTION_REQ_STATUS:
					{
						var status = parseInt(data[8]);
						rheaLog ("RHEA_EVENT_SELECTION_REQ_STATUS [" +status +"]");
						me.onEvent_selectionReqStatus(status);
					}
					break;
				}
				
				return;
			}
		}

		//è arrivato qualcosa di strano
		rheaLog ("Rhea::RCV [len=" +data.length +"] [" +utf8ArrayToStr(data) +"]");
		
	}
}


/***********************************************************
 * sendBinary
 *
 * invia tramite websocket un buffer di byte.
 * Il parametro [buffer] deve essere di tipo Uint8Array
 */
Rhea.prototype.sendBinary = function (buffer, byteStart, lengthInBytes)
{
	var toSend = new DataView(buffer.buffer, byteStart, lengthInBytes);
	this.websocket.send(toSend);
}

/***********************************************************
 * sendGPUCommand
 *
 *  invia un comando alla GPU.
 *	Ritorna una promise se [bReturnAPromise]==1
 *	Un comando è composto da un commandChar (ovvero un carattere ASCII) e un buffer di byte a seguire
 */
Rhea.prototype.sendGPUCommand = function (commandChar, bufferArrayIN, bReturnAPromise, promiseTimeoutMSec)
{
	var requestID = 255;
	var iReq;
	
	if (bReturnAPromise)
	{
		//requestID deve essere un byte, e deve essere maggiore di 0.
		//Ho messo il limite a 200, ma è un valore arbitrario, non ha un senso specifico. Serve solo ad evitare di eccedere
		//la capienza di un byte. Potevo usare 255 come limite, ma non si sa mai che in futuro si voglia riservare specifici
		//requestID per specifiche richieste
		var requestID = this.nextAjaxRequestID++;
		if (this.nextAjaxRequestID > 200)	
			this.nextAjaxRequestID = 1;

		
		for (iReq=0; iReq<this.ajaxReceivedAnswerQ.length; iReq++)
		{
			if (this.ajaxReceivedAnswerQ[iReq] == null)
			{
				this.ajaxReceivedAnswerQ[iReq] = new RheaAjaxAnswer (requestID);
				break;
			}
		}

		if (iReq >= this.ajaxReceivedAnswerQ.length)
		{
			rheaLog ("Rhea::GPUCommand => too many request");
			return new Promise ( function(resolve, reject) { reject("Rhea::GPUCommand => too many request")} );
		}
	} 	

	//prepara il buffer da inviare
	var bufferLen = 0;
	if (null != bufferArrayIN)
		bufferLen = bufferArrayIN.length;
	var bytesNeeded = 	1 +				//# 
						1 +				//requestID
						1 +				//char del comando
						2 +				//lunghezza di [bufferArrayIN] espressa come byte alto, byte basso
						bufferLen +		//[bufferArrayIN]
						2;				//cheksum espressa come byte alto, byte basso
	
	var buffer = new Uint8Array(bytesNeeded);
		var t = 0;
		buffer[t++] = 35; //'#'
		buffer[t++] = commandChar.charCodeAt(0);
		buffer[t++] = requestID;
		
		buffer[t++] = ((bufferLen & 0xFF00) >> 8);
		buffer[t++] = (bufferLen & 0x00FF);
		for (var i=0; i<bufferLen; i++)
			buffer[t++] = bufferArrayIN[i];

		//checksum
		var ck = 0;
		for (var i=0; i<t; i++)
			ck += buffer[i];

		buffer[t++] = ((ck & 0xFF00) >> 8);
		buffer[t++] = (ck & 0x00FF);

	//invia
	this.sendBinary(buffer, 0, t);
	
	if (bReturnAPromise == 0)
		return;


	var me = this;
	return new Promise( function(resolve, reject) 
					{
						var timeoutMs = promiseTimeoutMSec;
						var check = function()	{
												//console.warn('checking')
												if (me.ajaxReceivedAnswerQ[iReq].rcv != null)
												{
													resolve(me.ajaxReceivedAnswerQ[iReq].rcv);
													me.ajaxReceivedAnswerQ[iReq] = null;
												}
												else if ((timeoutMs -= 100) < 0)
												{
													reject ("timed out [reqID:" +me.ajaxReceivedAnswerQ[iReq].requestID +"][pos:" +iReq +"]");
													me.ajaxReceivedAnswerQ[iReq] = null;
												}
												else
													setTimeout(check, 100);
											}

						setTimeout(check, 100);
					});			
};

/***********************************************************
 * ajax
 *
 * 	Simula il meccanismo di ajax utlizzando il websocket come layer di trasporto.
 *	Ritorna una promise.
 *	Esempio di una richiesta con commandString="time" e vari parametri accessori
		rhea.ajax ("time", 
						{
							"name" : "pippo",
							"number" : i
						})
			.then( function(result) 
			{
				var obj = JSON.parse(result);
				alert (obj.numero);
			})
			.catch( function(result)
			{
				alert ("AJAX::error => " +result);
			});
 */
Rhea.prototype.ajax = function(commandString, plainJSObject)
{
	return this.ajaxWithCustomTimeout(commandString, plainJSObject, 2000);
};
Rhea.prototype.ajaxWithCustomTimeout = function(commandString, plainJSObject, timeoutMSec)
{
	if (plainJSObject=="")
		plainJSObject="{}";
	var jo = JSON.stringify(plainJSObject);

	var bytesNeeded = 	1 							//lenght di [commandString]
						+ commandString.length		//[commandString]
						+ 2							//lenght di [plainJSObject]
						+ jo.length					//[plainJSObject]

	var buffer = new Uint8Array(bytesNeeded);
		var t = 0;
		buffer[t++] = (commandString.length & 0x00FF);
		for (var i=0; i<commandString.length; i++)
			buffer[t++] = commandString.charCodeAt(i);

		buffer[t++] = ((jo.length & 0xFF00) >> 8);
		buffer[t++] = (jo.length & 0x00FF);
		for (var i=0; i<jo.length; i++)
			buffer[t++] = jo.charCodeAt(i);

	return this.sendGPUCommand ("A", buffer, 1, timeoutMSec);
};


/*********************************************************
 * requestGPUEvent
 *
 *	invia una richiesta alla GPU in modo che questa, a sua volta, invii un messaggio
 *	per scatenare l'evento indicato
 *	
 * 	Il parametro eventTypeID può valere una delle const che trovi ad inizio file e il cui nome inizia con RHEA_EVENT_
 */
Rhea.prototype.requestGPUEvent = function (eventTypeID)
{
	var buffer = new Uint8Array(1);
	buffer[0] = eventTypeID;
	this.sendGPUCommand ("E", buffer, 0, 0);
}


