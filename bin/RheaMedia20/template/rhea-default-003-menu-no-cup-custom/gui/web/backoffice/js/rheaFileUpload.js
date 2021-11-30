/*********************************************************
 * RheaFileUpload
 *
 * [rawData] è un ArrayBuffer contenente l'intero file da uppare
 */
function RheaFileUpload (rawData, usage, fileName, userValue, callback_onStart, callback_onProgress, callback_onEnd)
{
	//console.log ("RheaFileUpload, usage[" +usage +"]");
	
	this.rawDataAsUint8 = new Uint8Array(rawData);
	this.packetNum = 0;
	this.smuTransfUID = 0;
	this.packetSizeInBytes = 0;
	this.numPacketInAChunk = 0;
	this.userValue = userValue;
	this.appTransfUID = this.priv_generateUID();
	this.fileName = fileName;
	
	this.callback_onStart = callback_onStart;
	this.callback_onProgress = callback_onProgress;
	this.callback_onEnd = callback_onEnd;
	
	this.priv_requestUpload(usage);
}

//*********************************************************
RheaFileUpload.prototype.priv_generateUID = function()
{
	var a = parseInt(1 + Math.floor(Math.random() * 254));
	var b = parseInt(1 + Math.floor(Math.random() * 254));
	var c = parseInt(1 + Math.floor(Math.random() * 254));
	var d = parseInt(1 + Math.floor(Math.random() * 254));
	return ((a<<24) | (b<<16) | (c<<8) | d);
}

//*********************************************************
RheaFileUpload.prototype.priv_requestUpload = function(usage)
{
	var buffer = new Uint8Array(280);
	var ct = 0;
	
	var sData0x01;
	buffer[ct++] = 0x01;			//u8 opcode;
	buffer[ct++] = usage.length;		//u8 usageLen;
	
	//u16 packetSizeInBytes;
	rheaAddU16ToUint8Buffer(buffer, ct, 1024);
	ct+=2;

	//u32 fileSizeInBytes;
	//rheaAddU32ToUint8Buffer(buffer, ct, this.rawDataAsUint8.length);
	rheaAddU32ToUint8Buffer(buffer, ct, this.rawDataAsUint8.length);
	ct+=4;
	
	//u32 appTransfUID;
	rheaAddU32ToUint8Buffer(buffer, ct, this.appTransfUID);
	ct+=4;
	
	//u8 numPacketInAChunk;	
	buffer[ct++] = 24;

	//char	usage[MAX_USAGE_LEN];
	for (var i=0; i<usage.length; i++)
		buffer[ct+i] = usage.charCodeAt(i);
	buffer[ct+usage.length] = 0;
	ct+=255;
		
	rhea.sendGPUCommand ('F', buffer.subarray(0,ct), 0, 0);
}

//*********************************************************
RheaFileUpload.prototype.priv_on0x02 = function (sData0x02)
{
	//evento file transfer iniziato
	if (this.callback_onStart != null && typeof this.callback_onStart === "function")
		this.callback_onStart (this.fileName, this.userValue);
	
	if (sData0x02.reason_refused != 0)
	{
		//evento, fine del trasferimento con codice di errore
		if (this.callback_onEnd != null && typeof this.callback_onEnd === "function")
			this.callback_onEnd (this.fileName, this.userValue, sData0x02.reason_refused);
		return 0;		
	}
	
	this.packetSizeInBytes = sData0x02.packetSizeInBytes;
	this.numPacketInAChunk = sData0x02.numPacketInAChunk;
	this.smuTransfUID = sData0x02.smuTransfUID;
	
	
	//calcolo il num totale di pacchetti che si dovranno inviare
	this.numOfPacketToBeSentInTotal = Math.floor(this.rawDataAsUint8.length / this.packetSizeInBytes);
	if (this.numOfPacketToBeSentInTotal* this.packetSizeInBytes < this.rawDataAsUint8.length)
		this.numOfPacketToBeSentInTotal++;	

	
	//invio il primo blocco
	this.priv_sendChunkOfPackets();
	return 1;
}


//*********************************************************
RheaFileUpload.prototype.priv_on0x04 = function (sData0x04)
{
	//SMU ha accettato un pacchetto, aggiorno il mio stato
	this.packetNum = sData0x04.packetNumAccepted + 1;


	//genero l'evento
	var totalFileSize = this.rawDataAsUint8.length;
	var toraBytesTransferred = this.packetSizeInBytes * this.packetNum;
	if (null != this.callback_onProgress && typeof this.callback_onProgress === "function")
		this.callback_onProgress(this.fileName, this.userValue, toraBytesTransferred, totalFileSize);


	//vediamo se era l'ultimo
	if (this.packetNum >= this.numOfPacketToBeSentInTotal)
	{
		//ho finito!!
		if (null != this.callback_onEnd && typeof this.callback_onEnd === "function")
			this.callback_onEnd(this.fileName, this.userValue, 0);
		return 0;
	}

	//invio il prossimo blocco
	this.priv_sendChunkOfPackets();
	return 1;
}

//*********************************************************
RheaFileUpload.prototype.priv_sendChunkOfPackets = function ()
{
	var toSendBuffer = new Uint8Array(this.packetSizeInBytes+16);
	var iPacket = this.packetNum;
	var fileOffset = iPacket * this.packetSizeInBytes;
	var nBytesLeft = this.rawDataAsUint8.length - fileOffset;
	
	var nPacket = this.numPacketInAChunk;
	var chunkSeq = 1;
	while (nPacket-- && nBytesLeft)
	{
		var nByteToRead = this.packetSizeInBytes;
		if (nBytesLeft < nByteToRead)
		{
			nByteToRead = nBytesLeft;
			nBytesLeft = 0;
		}
		else
			nBytesLeft -= nByteToRead;

		var ct = 0;	
		toSendBuffer[ct++] = 0x03; //opcode
		rheaAddU32ToUint8Buffer(toSendBuffer, ct, this.smuTransfUID);
		ct+=4;
		
		rheaAddU32ToUint8Buffer(toSendBuffer, ct, iPacket);
		ct+=4;
		
		toSendBuffer[ct++] = chunkSeq;
		
		//console.log ("off[" +fileOffset +"], nbytes[" +nByteToRead +"]");
		var tempBuffer = this.rawDataAsUint8.subarray(fileOffset,fileOffset+nByteToRead);
		for (var i=0; i<nByteToRead; i++)
			toSendBuffer[ct++] = tempBuffer[i];
		tempBuffer = null;

		rhea.sendGPUCommand ('F', toSendBuffer.subarray(0,ct), 0, 0);

		iPacket++;
		chunkSeq++;
		fileOffset += nByteToRead;
	}
}