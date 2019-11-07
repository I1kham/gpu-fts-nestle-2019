/********************************************************
 * DA3
 *
 */
var DA3_BLOCK_SIZE = 64;


/*
	machineType indica se la macchina è ESPRESSO o INSTANT.
	machineModel è un numero che indica il modello della macchina (BL, Fusion...)
	In teoria queste informazioni sono nel DA3, in pratica non è detto che quello che trovi nel DA3 rappresenti l'attuale macchina.
	E' necessario quindi chiedere alla CPU tipo e modello e fare finta che queste informazioni ricevute dalla CPU siano davvero nel DA3
*/
function DA3(machineType, machineModel)
{
	this.da3_original = null;
	this.da3_current = null;
	this.da3_filesize = 0;
	this.machineType = machineType;
	this.machineModel = machineModel;
}


/********************************************************
 * load
 */
DA3.prototype.load = function ()
{
	console.log ("DA3::load() => mtype[" +this.machineType +"] mmodel[" +this.machineModel +"]");
	rhea.filetransfer_startDownload ("da3", this, DA3_load_onStart, DA3_load_onProgress, DA3_load_onEnd);
}

function DA3_load_onStart(userValue)			{ }
function DA3_load_onProgress()					{ }
function DA3_load_onEnd (theDa3, reasonRefused, obj)
{
	if (reasonRefused != 0)
	{
		console.log ("DA3_load_onEnd: error, reason[" +reasonRefused +"]");

		alert ("error downloading da3");
		onDA3Loaded();
		return;
	}
	
	//console.log ("da3_load_onEnd: succes. File size[" +obj.fileSize +"]");
	theDa3.da3_original = new Uint8Array(obj.fileSize);
	theDa3.da3_current = new Uint8Array(obj.fileSize);
	theDa3.da3_filesize = parseInt(obj.fileSize);
	for (var i=0; i<obj.fileSize; i++)
		theDa3.da3_original[i] = theDa3.da3_current[i] = obj.fileBuffer[i];
		
	//overload di machineType e modello
	theDa3.da3_original[9465] = theDa3.da3_current[9465] = theDa3.machineType;
	theDa3.da3_original[9466] = theDa3.da3_current[9466] = theDa3.machineModel;
	onDA3Loaded();
}

DA3.prototype.isInstant = function ()			{ if (parseInt(this.da3_current[9465]) == 0) return 1; return 0; }
DA3.prototype.isEpresso = function ()			{ if (parseInt(this.da3_current[9465]) > 0) return 1; return 0; }
DA3.prototype.getModelCode = function ()		{ return parseInt(this.da3_current[9466]); }
DA3.prototype.getNumProdotti = function ()		{ if (this.isEpresso()) return 6; else return 10; }



/********************************************************
 * compare
 *	Ritorna un array con i numeri dei blocchi da uppare
 */
DA3.prototype.compare = function ()
{
	var retList = [];
	//console.log ("DA3::compare() => begin");
	
	var nBlock = parseInt(Math.floor(this.da3_filesize / DA3_BLOCK_SIZE));
	//console.log ("DA3::compare() => da3_filesize[" +this.da3_filesize +"]");
	//console.log ("DA3::compare() => nBlock[" +nBlock +"]");
	for (var block=0; block<nBlock; block++)
	{
		var bChanged = 0;
		var ct = block * DA3_BLOCK_SIZE;
		for (var i=0; i<DA3_BLOCK_SIZE; i++)
		{
			if (this.da3_current[ct] != this.da3_original[ct])
			{
				retList.push(block);
				break;				
			}
			ct++;
		}
	}
	
	//console.log ("DA3::compare() => end");
	//console.log (retList);
	return retList;
}

DA3.prototype.storeBlock = function (uno_di_n, num_tot_block, blockNum)
{
	//console.log ("DA3::sendUpdatedBlock() => sending block [" +uno_di_n +"]/[" +num_tot_block +"] [" +blockNum +"]");
	return rhea.sendPartialDA3AndReturnAPromise (uno_di_n, num_tot_block, blockNum, this.da3_current, blockNum * DA3_BLOCK_SIZE);
}

DA3.prototype.copyBlockToOriginal = function (blockNum)
{
	var ct = blockNum * DA3_BLOCK_SIZE;
	for (var i=0; i<DA3_BLOCK_SIZE; i++)
	{
		this.da3_original[ct] = this.da3_current[ct];
		ct++;
	}
}

DA3.prototype.read16  = function (posIN)
{ 
	var pos = parseInt(posIN);
	var ret = this.da3_current[pos]; 
	ret += 256 * this.da3_current[pos+1]; 
	//console.log("DA3 read@" +pos +"=" + ret);
	return ret; 
}
DA3.prototype.write16 = function (pos, value)
{ 
	//console.log("DA3 write@" +pos +"=" + value);
	var v = parseInt(value); 
	this.da3_current[pos++] = (value & 0x00FF); 
	this.da3_current[pos]   = ((value & 0xFF00) >>8); 
}
