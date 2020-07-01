/***************************************************************
 * UINumber
 
 *	attributi consentiti:
 *
 *		data-numfigures="2"					=> numero totale di cifre da visualizzare
 *			opzionale data-value="7"		=> valore da visualizzare (0 se non indicato)
 *			opzionale data-da3="xxx"		=> locazione in memoria dalla quale leggere/scrivere il numero
 *			opzionale data-da3bit="4|8|9|16"	=> legge 4,8,9,16,32 bit dal da3, default 16.
												NEl caso di 9 bit, è necessaria anche l'opzione data-da3bit9
 *			opzionale data-da3bit9="a,b"	=>  indica dove andare a prendere il nono bit.
												il nono viene preso dalla posizione [da3pos+a] al bit [b] con b che va da 0 a 7
 *			opzionale data-min				=> default = 0
 *			opzionale data-max				=> default = 999999999
 *			opzionale data-decimal			=> numero di cifre decimali dopo il "." 
												ATTENZIONE che UINumber ritorna sempre e cmq un numero intero, il decimale è solo un fatto estetico
*			opzionale data-um="sec."		=> scrive l'unità di misura a destra dell'ultimo numero
 */
var UINUMBER_TOP_OFFSET = 10;
var UINUMBER_NUM_HEIGHT = 56;
function UINumber (parentID, childNum, node)
{
	this.flash_alpha_0_1 = 0;
	this.valueMin = parseInt(UIUtils_getAttributeOrDefault(node, "data-min", "0"));
	this.valueMax = parseInt(UIUtils_getAttributeOrDefault(node, "data-max", "999999999"));
	
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}

	this.numCifre = parseInt(node.getAttribute("data-numfigures"));
	if (this.numCifre < 1) this.numCifre = 1;
	else if (this.numCifre > 12) this.numCifre = 12;
	
	//binding a da3
	this.da3offset = 0;
	this.da3bit = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3bit", "16"));	
	if (this.da3bit == 9)
		this.da3bit9 = UIUtils_getAttributeOrDefault(node, "data-da3bit9", "")
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	this.numDecimalAfterPoint = parseInt(UIUtils_getAttributeOrDefault(node, "data-decimal", "0"));
	
	//genero l'HTML
	this.mousedown_relY = [];
	this.clickPosX = [];
	this.clickPosY = [];
	this.clickStartTimeMSec	 = [];
	
	this.priv_buildHTML(node);
	
	//value
	if (this.da3Pos >= 0)
		this.loadFromDA3(da3);
	else
		this.setValue(parseInt(UIUtils_getAttributeOrDefault(node, "data-value", "0")));
}

UINumber.prototype.priv_buildHTML = function(nodeIN)
{
	var node = nodeIN;
	if (null == node)
		node = document.getElementById(this.id);
		

	var html = "";
	for (var i=0; i<this.numCifre; i++)
	{
		this.mousedown_relY[i] = 0;
		this.clickPosX[i] = 0;
		this.clickPosY[i] = 0;
		this.clickStartTimeMSec[i] = 0;
		
		html += this.priv_getHTMLForAFigure(i);
		
		if (this.numDecimalAfterPoint>0 && i == this.numCifre - this.numDecimalAfterPoint -1)
			html += "<div class='UINumber3Container' style='width:15px;'><div class='UINumber3DecimalPoint'>.</div></div>";
	}
	
	var um = UIUtils_getAttributeOrDefault(node,"data-um", "");
	if (um != "")
	{
		var idUM = this.id +"_um";
		html += "<div id='" +idUM +"' class='UINumber3Container' style='width:auto; height:auto; font-size:1.0em;'>" +um +"</div>";
	}
	
	node.innerHTML = html;
	
	if (null == nodeIN)
		this.bindEvents();
}


UINumber.prototype.flashBackground = function (valueToSetAtTheEndOfAnimation)
{
	this.flash_valueToSetAtTheEndOfAnimation = valueToSetAtTheEndOfAnimation;
	if (this.flash_alpha_0_1 > 0)
	{
		this.flash_alpha_0_1 = 1;
		return;
	}
	
	this.flash_alpha_0_1 = 1;
	this.priv_do_flashBackground();
	
}
UINumber.prototype.priv_do_flashBackground = function ()
{
	var c = Math.floor(255.0 * this.flash_alpha_0_1);
	var targetCol = "rgb(" +c +",0,0)";
	for (var i=0; i<this.numCifre; i++)
	{
		var idStrip = this.id +"_fig" +i;
		var idContainer = idStrip+"_cnt";
		rheaSetElemBackgroundColor (rheaGetElemByID(idContainer), targetCol);
	}
	
	if (this.flash_alpha_0_1 > 0)
	{
		this.flash_alpha_0_1 -= 0.05;
		var me = this;
		
		if (this.flash_alpha_0_1 < 0.2)
			this.setValue(this.flash_valueToSetAtTheEndOfAnimation);
		
		setTimeout ( function() { me.priv_do_flashBackground(); }, 40 );
	}
	else 
		this.setValue(this.flash_valueToSetAtTheEndOfAnimation);
}

UINumber.prototype.priv_getHTMLForAFigure = function(i)
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

UINumber.prototype.setDA3Offset = function (da3offset) 	{ this.da3offset = da3offset; }
UINumber.prototype.loadFromDA3 = function(da3)			{ var loc = this.da3Pos + this.da3offset; if (loc >= 0) this.setValue (this.priv_getValueFromDA3(da3,loc)); }
UINumber.prototype.saveToDA3 = function(da3)		
{ 
	var loc = this.da3Pos + this.da3offset; 
	if (loc >= 0) 
	{
		switch (this.da3bit)
		{
			case 4:		da3.write4(loc, this.getValue()); break;
			case 8:		da3.write8(loc, this.getValue()); break;
			case 9:
				var val = parseInt(this.getValue());
				var val8 = (val & 0xFF);
				var bit9 = 0;
				if ( (val & 0x0100) != 0)
					bit9=1;
				da3.write8(loc, val8);
				
				var e = this.da3bit9.split(",");
				var v2 = parseInt(da3.read8(loc + parseInt(e[0])));
				var mask = parseInt((0x01 << parseInt(e[1])));
				if (bit9==1)
					v2 = v2 | mask;
				else
					v2 = v2 & (~mask);
				da3.write8(loc + parseInt(e[0]), v2);
				break;
				
			default: 	da3.write16(loc, this.getValue()); break;
			case 32: 	da3.write32(loc, this.getValue()); break;
				
		}		
	}
}

UINumber.prototype.priv_getValueFromDA3 = function(da3, loc)
{
	switch (this.da3bit)
	{
		case 4:		return da3.read4(loc);
		case 8:		return da3.read8(loc);
		case 9:
			var v = parseInt(da3.read8(loc));
			var e = this.da3bit9.split(",");
			var v2 = da3.read8(loc + parseInt(e[0]));
			var mask = parseInt((0x01 << parseInt(e[1])));
			if ( (v2 & mask) != 0)
				v +=256;
			return v;
		default: 	return da3.read16(loc);
		case 32:	return da3.read32(loc);
	}
}

UINumber.prototype.bindEvents = function()
{
	for (var i=0; i<this.numCifre; i++)
		this.priv_bindEvents(i);
}

UINumber.prototype.priv_bindEvents = function(iCifra)
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
		//console.log ("UINumber::mousedown => rely[" +me.mousedown_relY[iCifra]+"]");
	}, 
	true);

	node.addEventListener("mouseup", function (ev)
	{
		if (me.flash_alpha_0_1 > 0)
			return;
		
		var timeElapsedMSec = parseInt(ev.timeStamp - me.clickStartTimeMSec[iCifra]);
		var xdiff = parseInt(Math.abs(me.clickPosX[iCifra] - ev.clientX));
		var ydiff = parseInt(Math.abs(me.clickPosY[iCifra] - ev.clientY));
			
		me.clickStartTimeMSec[iCifra] = -1;
		if (timeElapsedMSec <650 && xdiff <60 &&ydiff<40)
		{
			var dStrip = document.getElementById(idStrip);
			var whichNum = parseInt(dStrip.getAttribute("data-value"));
			var bVolevoIncrementare = 0;
			if (me.mousedown_relY[iCifra] < 0)
			{
				bVolevoIncrementare = 1;
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
			
			var curValue = me.getValue();
			dStrip.setAttribute("data-value", whichNum);		
			var newValue = me.getValue();

			//per assicurarmi di stare all'interno di valueMin/valueMax...
			//me.setValue(me.getValue());
			
			if (newValue < me.valueMin || newValue > me.valueMax) 
			{
				console.log ("cur[" +curValue +"], new[" +newValue +"]");
				if (bVolevoIncrementare)
				{
					me.setValue(me.valueMax);
					//me.flashBackground(curValue);
					me.flashBackground(me.valueMax);
				}
				else
				{
					me.setValue(me.valueMin);
					me.flashBackground(me.valueMin);
				}
				
			}
			else
				me.setValue(newValue);
		}
	}, 
	true);	
}

UINumber.prototype.setDecimals = function(n)
{
	if (this.numDecimalAfterPoint == n)
		return;
	this.numDecimalAfterPoint = n;
	
	var v = this.getValue();
	this.priv_buildHTML(null);
	this.setValue(v);
	
}

UINumber.prototype.getValue = function()			
{
	var ret = "";
	for (var i=0; i<this.numCifre; i++)
	{
		var idStrip = this.id +"_fig" +i;
		var dStrip = document.getElementById(idStrip);
		ret = ret + dStrip.getAttribute("data-value");
	}
	return parseInt(ret);
} 

UINumber.prototype.priv_clampToLimit = function(v)
{
	var value = parseInt(v);
	if (value==null || value=="")
		value = 0;
	
	if (value < this.valueMin) value = this.valueMin;
	if (value > this.valueMax) value = this.valueMax;

	return value;
}

UINumber.prototype.setValue = function(v)
{
	var value = this.priv_clampToLimit(v);
	
	var s = value.toString();
	while (s.length < this.numCifre)
		s ="0" + s;
	if (s.length>this.numCifre)
		s = s.substr(0,this.numCifre);
	
	for (var i=0; i<this.numCifre; i++)
	{
		var num = parseInt(s.substr(i,1));
		var idStrip = this.id +"_fig" +i;
		var dStrip = document.getElementById(idStrip);
		dStrip.setAttribute("data-value", num);
		dStrip.innerHTML = "<p>" +num +"</p>";
	}
}

UINumber.prototype.changeUM = function(v)
{
	var idUM = this.id +"_um";
	document.getElementById(idUM).innerHTML =v;
}