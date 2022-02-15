/*********************************************************
 * RheaFileDownload
 *
 */
function RheaFileDownload (usage, userValue, callback_onStart, callback_onProgress, callback_onEnd)
{
	this.userValue = userValue;
	this.callback_onStart = callback_onStart;
	this.callback_onProgress = callback_onProgress;
	this.callback_onEnd = callback_onEnd;	
	
	this.packetSizeInBytes = 0;
	this.numPacketInAChunk = 0;
	this.smuTransfUID = 0;
	this.fileSize = 0;
	this.numOfPacketToBeRcvInTotal = 0;
	this.lastGoodPacket = 0xFFFFFFFF;
	this.appTransfUID = this.priv_generateUID();
	this.fileBuffer = null;
	this.fileBufferCT = 0;
	this.priv_requestDownload(usage);
}

//*********************************************************
RheaFileDownload.prototype.priv_generateUID = function()
{
	var a = parseInt(1 + Math.floor(Math.random() * 254));
	var b = parseInt(1 + Math.floor(Math.random() * 254));
	var c = parseInt(1 + Math.floor(Math.random() * 254));
	var d = parseInt(1 + Math.floor(Math.random() * 254));
	return ((a<<24) | (b<<16) | (c<<8) | d);
}

//*********************************************************
RheaFileDownload.prototype.priv_requestDownload = function(usage)
{
	var buffer = new Uint8Array(usage.length+6);
	var ct = 0;
	
	var sData0x51;
	buffer[ct++] = 0x51;			//u8 opcode;
	buffer[ct++] = usage.length;	//u8 usageLen;
	
	
	//u32 appTransfUID;
	rheaAddU32ToUint8Buffer(buffer, ct, this.appTransfUID);
	ct+=4;

	//char	usage[MAX_USAGE_LEN];
	for (var i=0; i<usage.length; i++)
		buffer[ct+i] = usage.charCodeAt(i);
	ct+=usage.length
	buffer[ct++] = 0;
		
	rhea.sendGPUCommand ('F', buffer.subarray(0,ct), 0, 0);
}

//*********************************************************
RheaFileDownload.prototype.priv_on0x52 = function (sData0x52)
{
	//evento file transfer iniziato
	if (this.callback_onStart != null && typeof this.callback_onStart === "function")
		this.callback_onStart (this.userValue);
	
	if (sData0x52.reason_refused != 0)
	{
		//evento, fine del trasferimento con codice di errore
		if (this.callback_onEnd != null && typeof this.callback_onEnd === "function")
			this.callback_onEnd (this.userValue, sData0x52.reason_refused, null);
		return 0;		
	}

	this.packetSizeInBytes = sData0x52.packetSizeInBytes;
	this.smuTransfUID = sData0x52.smuTransfUID;
	this.fileSize = sData0x52.fileSize;
	this.numPacketInAChunk = sData0x52.numPacketInAChunk;
	this.lastGoodPacket = 0xFFFFFFFF;
	this.fileBuffer = new Uint8Array(this.fileSize);
	
	//calcolo il num totale di pacchetti che si dovranno ricevere in tutto
	this.numOfPacketToBeRcvInTotal = Math.floor(this.fileSize / this.packetSizeInBytes);
	if (this.numOfPacketToBeRcvInTotal* this.packetSizeInBytes < this.fileSize)
		this.numOfPacketToBeRcvInTotal++;	

	
	//invio una ack per far partire il download
	this.priv_sendACK(this.lastGoodPacket);
	return 1;
}

//*********************************************************
RheaFileDownload.prototype.priv_sendACK = function(packetNumAccepted)
{
	var toSendBuffer = new Uint8Array(16);
	var ct=0;
	toSendBuffer[ct++] = 0x54; //opcode
	
	rheaAddU32ToUint8Buffer(toSendBuffer, ct, this.smuTransfUID);
	ct+=4;

	rheaAddU32ToUint8Buffer(toSendBuffer, ct, packetNumAccepted);
	ct+=4;
		
	rhea.sendGPUCommand ('F', toSendBuffer.subarray(0,ct), 0, 0);
	
}

/*********************************************************
 * ritorna > 0 se il pacchetto è da memorizzare nel buffer nel qual caso il num ritornato corrisponde al num di bytes da copiare
 * 0 se è da scartare
 * <0 se è l'ultimo pacchetto
 */
RheaFileDownload.prototype.priv_on0x53 = function (sData0x53)
{
	var expectedPacketNum = this.lastGoodPacket + 1;
	if (expectedPacketNum > 0xffffffff)
		expectedPacketNum = 0;


	if (sData0x53.packetNumReceived == expectedPacketNum)
	{
		this.lastGoodPacket = sData0x53.packetNumReceived;

		if (this.lastGoodPacket + 1 >= this.numOfPacketToBeRcvInTotal)
		{
			var lastPacketSize = this.fileSize - this.lastGoodPacket * this.packetSizeInBytes;
			this.priv_sendACK(this.lastGoodPacket);
			return -lastPacketSize;
		}

		if (sData0x53.chunkSeq == this.numPacketInAChunk)
			this.priv_sendACK(this.lastGoodPacket);
		return this.packetSizeInBytes;
	}

	this.priv_sendACK(this.lastGoodPacket);
	return 0;
}
