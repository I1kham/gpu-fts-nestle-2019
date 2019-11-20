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
function DA3(machineType, machineModel, isInduzione)
{
	console.log ("DA3: machineType[" +machineType +"], isInduzione[" +isInduzione +"]");
	this.da3_original = null;
	this.da3_current = null;
	this.da3_filesize = 0;
	this.machineType = machineType;
	this.machineModel = machineModel;
	this.bInduzione = parseInt(isInduzione);
}


/********************************************************
 * load
 */
DA3.prototype.load = function ()
{
	//console.log ("DA3::load() => mtype[" +this.machineType +"] mmodel[" +this.machineModel +"]");
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
	}
	else
	{
		//console.log ("da3_load_onEnd: succes. File size[" +obj.fileSize +"]");
		theDa3.da3_original = new Uint8Array(obj.fileSize);
		theDa3.da3_current = new Uint8Array(obj.fileSize);
		theDa3.da3_filesize = parseInt(obj.fileSize);
		for (var i=0; i<obj.fileSize; i++)
			theDa3.da3_original[i] = theDa3.da3_current[i] = obj.fileBuffer[i];
			
		//overload di machineType e modello
		theDa3.da3_original[9465] = theDa3.da3_current[9465] = theDa3.machineType;
		theDa3.da3_original[9466] = theDa3.da3_current[9466] = theDa3.machineModel;
	}
	
	onDA3Loaded();
}

DA3.prototype.isInduzione = function ()			{ return this.bInduzione; }
DA3.prototype.isInstant = function ()			{ if (parseInt(this.da3_current[9465]) == 0) return 1; return 0; }
DA3.prototype.isEspresso = function ()			{ if (parseInt(this.da3_current[9465]) > 0) return 1; return 0; }
DA3.prototype.getNumMacine = function()			{ if (this.isInstant()) return 0;  return parseInt(this.da3_current[9465]); }
DA3.prototype.getModelCode = function ()		{ return parseInt(this.da3_current[9466]); }
DA3.prototype.getNumProdotti = function ()		{ if (this.isEspresso()) return 6; else return 10; }

DA3.prototype.isMotorCalibrated = function (motor)	{ return (this.getCalibFactorGSec(motor) != 0); }

DA3.prototype.priv_getLocationForCalibFactor = function (motor)
{
	if (this.isInstant())
	{
		if (motor>=1 && motor<=6)
			return (9693 + (motor-1)*2);
	}
	else
	{
		if (motor>=2 && motor<=6)
			return (9693 + (motor-1)*2);
		else if (motor==11) //macina 1
			return 9693;
		else if (motor==12) //macina 2
			return 9705;
	}	
	return 0;
}

DA3.prototype.setCalibFactorGSec = function (motor, v)		
{ 
	var loc = this.priv_getLocationForCalibFactor(parseInt(motor)); 
	//console.log ("DA3::setCalibFactorGSec motor[" +motor +"] v[" +v +"] loc[" +loc +"]");
	if (loc>0) 
		this.write16(loc, parseInt(v));
	
	//macina 1 va salvata anche qui
	if (motor==11)
		this.write16(7534, parseInt(v));
	
	//macina 2 va salvata anche qui
	if (motor==12)
		this.write16(7546, parseInt(v));
}
DA3.prototype.getCalibFactorGSec = function (motor)			
{ 
	var loc = this.priv_getLocationForCalibFactor(parseInt(motor)); 
	//console.log ("DA3::getCalibFactorGSec motor[" +motor +"] loc[" +loc +"]");
	if (loc>0) 
		return this.read16(loc); return 0;
}


DA3.prototype.getImpulsi = function (motor)		
{ 
	if (motor==11)
		return this.read16(7560); //macina 1
	else if (motor==12)
		return this.read16(7564); //macina 2
	return 0;
	
}
DA3.prototype.setImpulsi = function (motor,v)	
{ 
	if (motor==11)
		this.write16(7560, parseInt(v)); //macina 1
	else if (motor==12)
		this.write16(7564, parseInt(v)); //macina 2
}


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
				//console.log ("da3:: diff @ pos[" +ct +"], old[" +this.da3_original[ct] +"], new[" +this.da3_current[ct] +"]");
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

DA3.prototype.read4  = function (posIN)
{ 
	var pos = parseInt(posIN);
	var ret = this.da3_current[pos]; 
	//console.log("DA3 read4@" +pos +"=" + ret);
	return (ret & 0x0F);
}
DA3.prototype.write4 = function (pos, value)
{ 
	var v = (parseInt(value) & 0x0F)
	var c = this.da3_current[pos] & 0xF0;
	c |= v;
	//console.log("DA3 write4@" +pos +"=" + v);
	if (pos == 146) console.log("DA3 write4@" +pos +"=[" + v +"], old[" +this.da3_current[pos] +"]");
	this.da3_current[pos] = c;
}

DA3.prototype.read8  = function (posIN)
{ 
	var pos = parseInt(posIN);
	var ret = this.da3_current[pos]; 
	//console.log("DA3 read8@" +pos +"=" + ret);
	return ret;
}
DA3.prototype.write8 = function (pos, value)
{ 
	var v = parseInt(value); 
	if (v<0) v = 0;
	if (v>255) v=255;
	//console.log("DA3 write8@" +pos +"=" + v);
	this.da3_current[pos] = v;
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
	var v = parseInt(value); 
	this.da3_current[pos++] = (v & 0x00FF); 
	this.da3_current[pos]   = ((v & 0xFF00) >>8); 
	if (pos == 146) console.log("DA3 write16@" +pos +"=" + v);
	//console.log("DA3 write16@" +pos +"=" + v);
}

DA3.prototype.read32  = function (posIN)
{ 
	var pos = parseInt(posIN);
	var ret = this.da3_current[pos] | (this.da3_current[pos+1]<<8) | (this.da3_current[pos+2]<<16) | (this.da3_current[pos+3]<<24);
	//console.log("DA3 read32" +pos +"=" + ret);
	return ret; 
}
DA3.prototype.write32 = function (pos, value)
{ 
	var v = parseInt(value); 
	this.da3_current[pos++] = (v & 0x000000FF); 
	this.da3_current[pos++] = ((v & 0x0000FF00) >>8); 
	this.da3_current[pos++] = ((v & 0x00FF0000) >>16); 
	this.da3_current[pos++] = ((v & 0xFF000000) >>24); 
	
	if (pos == 146) console.log("DA3 write16@" +pos +"=" + v);
	//console.log("DA3 write32@" +pos +"=" + v);
}

DA3.prototype.getPriceLocation = function (list1_2, price1_48)
{
	var N = 64;
	var LISTA1 = 6500;
	return LISTA1 + N*2*(list1_2 -1) + 2*(price1_48-1);
}
DA3.prototype.getTeaBagLocation = function (iSel) 	{ return 176 +(iSel-1)*100; }
DA3.prototype.getJugLocation = function (iSel) 		{ return 177 +(iSel-1)*100; }