/***************************************************************
 * UITime24
 
 *	attributi consentiti:
 *			opzionale data-value-hh="00"
 *			opzionale data-value-mm="00"
 *			opzionale data-da3="xxx"	=> locazione in memoria dalla quale leggere/scrivere il numero
											IL formato dell'ora è 16bit calcolato come h*100 + min
 *
 *	ATTENZIONE: questa è in pratica un "hack" della classe UINUmber per cui è necessario includere UINumber prima di questo file
 */
var UINUMBER_TOP_OFFSET = 10;
var UINUMBER_NUM_HEIGHT = 56;
function UITime24 (parentID, childNum, node, parentObj)
{
	this.parentObj = parentObj;
	
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}

	this.numCifre = 4;
	
	//binding a da3
	this.da3offset = 0;
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	//genero l'HTML
	this.mousedown_relY = [];
	this.clickPosX = [];
	this.clickPosY = [];
	this.clickStartTimeMSec	 = [];
	
	var html = "";
	for (var i=0; i<this.numCifre; i++)
	{
		this.mousedown_relY[i] = 0;
		this.clickPosX[i] = 0;
		this.clickPosY[i] = 0;
		this.clickStartTimeMSec[i] = 0;
		
		html += this.priv_getHTMLForAFigure(i);
		
		if (i==1)
			html += "<div class='UINumber3Container' style='width:15px; font-size:3.0em;'>:</div>";
	}
	node.innerHTML = html;

	//value
	if (this.da3Pos >= 0)
		this.loadFromDA3(da3);
	else
	{
		this.setValueHH(parseInt(UIUtils_getAttributeOrDefault(node, "data-value-hh", "0")));
		this.setValueMM(parseInt(UIUtils_getAttributeOrDefault(node, "data-value-mm", "0")));
	}
}

UITime24.prototype.priv_getHTMLForAFigure = function(i)
{
	var idStrip = this.id +"_fig" +i;
	var idContainer = idStrip+"_cnt";
	var idClicker = idStrip+"_clk";	
	var html =   "<div class='UINumber3Container' id='" +idContainer +"'>"
				+"	<div class='UINumber3Strip' id='" +idStrip +"'><p>0</p></div>"
				+"	<div class='UINumber3Border'>&nbsp;</div>"
				+"	<div style='position:absolute; top:3px; left:17px; z-index:22;'><img src='img/UINumberArrowUp-grey.png' draggable='false'></div>"
				+"	<div style='position:absolute; top:76px; left:17px; z-index:22;'><img src='img/UINumberArrowDown-grey.png' draggable='false'></div>"
				+"	<div id='" +idClicker +"' class='UINumber3Clicker'>&nbsp;</div>"
				+"</div>";
	return html;
}

UITime24.prototype.setDA3Offset = function (da3offset) { this.da3offset = da3offset; }
UITime24.prototype.loadFromDA3 = function(da3)
{ 
	var loc = this.da3Pos + this.da3offset; 
	if (loc >= 0) 
	{
		var v = da3.read16(loc);
		var hh = Math.floor(v / 100);
		var mm = v - Math.floor(hh*100);
		this.setValueHH (hh);
		this.setValueMM (mm);
	}
}

UITime24.prototype.saveToDA3 = function(da3)		
{ 
	var loc = this.da3Pos + this.da3offset; 
	if (loc >= 0) 
	{
		var v = parseInt(this.getValueHH()) * 100 + parseInt(this.getValueMM());
		da3.write16(loc, v);
	}
}

UITime24.prototype.bindEvents = function()
{
	for (var i=0; i<this.numCifre; i++)
		this.priv_bindEvents(i);
}

UITime24.prototype.priv_bindEvents = function(iCifra)
{
	var me = this;
	var idStrip = this.id +"_fig" +iCifra;
	var idClicker = idStrip+"_clk";

	var node = document.getElementById(idClicker);
	node.addEventListener("mousedown", function (ev)
	{
		var rect = ev.target.getBoundingClientRect();
		me.mousedown_relY[iCifra] = (ev.clientY - rect.top) - 42;
		me.clickPosX[iCifra] = ev.clientX;
		me.clickPosY[iCifra] = ev.clientY;
		me.clickStartTimeMSec[iCifra] = ev.timeStamp;		
		//console.log ("UITime24::mousedown => rely[" +me.mousedown_relY[iCifra]+"]");
	}, 
	true);

	node.addEventListener("mouseup", function (ev)
	{
		var timeElapsedMSec = parseInt(ev.timeStamp - me.clickStartTimeMSec[iCifra]);
		var xdiff = parseInt(Math.abs(me.clickPosX[iCifra] - ev.clientX));
		var ydiff = parseInt(Math.abs(me.clickPosY[iCifra] - ev.clientY));
			
		me.clickStartTimeMSec[iCifra] = -1;
		if (timeElapsedMSec <650 && xdiff <60 &&ydiff<40)
		{
			var dStrip = document.getElementById(idStrip);
			var whichNum = parseInt(dStrip.getAttribute("data-value"));
			if (me.mousedown_relY[iCifra] < 0)
			{
				whichNum++;
				if (whichNum > 9)
					whichNum = 0;
			}
			else
			{
				whichNum--;
				if (whichNum < 0)
					whichNum = 9;
			}
			dStrip.setAttribute("data-value", whichNum);		

			//console.log ("UITime24::mouseup => whichNum[" +whichNum+"]");
			
			//per assicurarmi di stare all'interno di valueMin/valueMax...
			me.setValueHH(me.getValueHH());
			me.setValueMM(me.getValueMM());
		}
	}, 
	true);	
}

UITime24.prototype.priv_setCifra = function(iFig, numIN)
{
	var num = parseInt(numIN);
	var idStrip = this.id +"_fig" +iFig;
	var dStrip = document.getElementById(idStrip);
	//dStrip.style.top = (UINUMBER_TOP_OFFSET - UINUMBER_NUM_HEIGHT*num) +"px";		
	dStrip.setAttribute("data-value", num);
	dStrip.innerHTML = "<p>" +num +"</p>";
}

UITime24.prototype.getValueHH = function()
{
	var ret = document.getElementById(this.id +"_fig0").getAttribute("data-value").toString();
	ret += document.getElementById(this.id +"_fig1").getAttribute("data-value").toString();
	return ret;	
}

UITime24.prototype.getValueMM = function()
{
	var ret = document.getElementById(this.id +"_fig2").getAttribute("data-value").toString();
	ret += document.getElementById(this.id +"_fig3").getAttribute("data-value").toString();
	return ret;
}

UITime24.prototype.getValue = function()	{ return this.getValueHH() +":" + getValueMM(); } 

UITime24.prototype.setValueHH = function(v)
{
	var vInt = parseInt(v);
	if (vInt<0) vInt = 0;
	if (vInt>23) vInt = 23;
	
	var value = vInt.toString();
	if (vInt < 10)
		value = "0" +value;
	
	this.priv_setCifra (0, value.substr(0,1));
	this.priv_setCifra (1, value.substr(1,1));
}

UITime24.prototype.setValueMM = function(v)
{
	var vInt = parseInt(v);
	if (vInt<0) vInt = 0;
	if (vInt>59) vInt = 59;
	
	var value = vInt.toString();
	if (vInt < 10)
		value = "0" +value;
	
	this.priv_setCifra (2, value.substr(0,1));
	this.priv_setCifra (3, value.substr(1,1));
}