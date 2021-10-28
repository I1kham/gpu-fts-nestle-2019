/***************************************************************
 * UIOption
 
 *	attributi consentiti:
 *
 *		data-option="value|caption|value|caption|....value|caption"
 *			opzionale data-selected="value"	=> indica l'opzione selezionata. Default "" == nessuna selezione
 *			opzionale data-da3="xxx"		=> locazione in memoria dalla quale leggere/scrivere il [data-selected]
 *			opzionale data-da3bit="4|8|16"	=> legge 4 o 8 bit dal da3, default 8
 *			opzionale data-onclick ="showPage('pageMaintenance')"
 */
function UIOption (parentID, childNum, node)
{
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}
	this.cssClass = node.getAttribute("class");
	
	this.jsOnClick = UIUtils_getAttributeOrDefault(node, "data-onclick", "");

	//elenco opzioni
	this.optionValue = [];
	this.optionCaption = [];
	this.clickPosX = [];
	this.clickPosY = [];
	this.clickStartTimeMSec = [];
	
	var e = node.getAttribute("data-option").split("|");
	var nOptions = 0;
	for (var i=0; i<e.length;)
	{
		this.optionValue[nOptions] = e[i++];
		this.optionCaption[nOptions] = e[i++];
		nOptions++;
	}
	
	//binding a da3
	this.allowDa3Save=1;
	this.da3offset = 0;
	this.da3bit = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3bit", "8"));
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	//inietto l'html
	this.priv_buildHTML();

	//opzione selezionata
	this.selectedOption = -1;
	if (this.da3Pos >= 0)
		this.loadFromDA3 (da3, this.da3Pos);
	else
		this.selectOptionByValue (UIUtils_getAttributeOrDefault(node, "data-selected", "-1"));
}

UIOption.prototype.priv_buildHTML = function ()
{
	const nOptions = this.optionValue.length;
	const cellSize = parseInt (100 / nOptions);
	var node = document.getElementById(this.id);
	
	var html = "<table class='" +this.cssClass +"'><tr>";
	for (var i=0; i<nOptions; i++)
	{
		var btnID = this.id +"_opt" +i;
		var css = "UIButton";
		html += "<td width='" +cellSize +"%'><div id='" +btnID +"' class='" +css +"' data-optnum='" +i +"'><p>" +this.optionCaption[i] +"</p></div></td>";
	}
	html += "</tr></table>";
	node.innerHTML = html;
}

UIOption.prototype.changeOptions = function (str)
{
	this.optionValue = [];
	this.optionCaption = [];
	
	var e = str.split("|");
	var nOptions = 0;
	for (var i=0; i<e.length;)
	{
		this.optionValue[nOptions] = e[i++];
		this.optionCaption[nOptions] = e[i++];
		nOptions++;
	}	
	this.priv_buildHTML();
	
	var n = this.selectedOption;
	this.selectedOption = -1;
	this.bindEvents();
}

UIOption.prototype.changeCaptionByIndex = function (i, msg)
{
	this.optionCaption[i] = msg;
	var btnID = this.id +"_opt" +i;
	var d = rheaGetElemByID(btnID);
	rheaSetElemHTML(d, "<p>" +this.optionCaption[i] +"</p>");
}

UIOption.prototype.dontSaveToDa3 = function () 					{ this.allowDa3Save = 0; }
UIOption.prototype.allowSaveToDa3 = function () 				{ this.allowDa3Save = 1; }
UIOption.prototype.setDA3Offset = function(da3offset)			{ this.da3offset = da3offset;}
UIOption.prototype.loadFromDA3 = function(da3)					
{ 
	var loc = this.da3Pos + this.da3offset; 
	if (loc >= 0) 
	{
		switch (this.da3bit)
		{
		case 4:		this.selectOptionByValue (da3.read4(loc));  break;
		default:	this.selectOptionByValue (da3.read8(loc));  break;
		case 16:	this.selectOptionByValue (da3.read16(loc)); break;
		}
	}
}	
UIOption.prototype.saveToDA3 = function(da3)					
{ 
	if (this.allowDa3Save == 0)
		return;
	var loc = this.da3Pos + this.da3offset; 
	if (loc >= 0) 
	{
		switch (this.da3bit)
		{
		case 4:		da3.write4(loc, this.getSelectedOptionValue()); break;
		default:	da3.write8(loc, this.getSelectedOptionValue()); break;
		case 16:	da3.write16(loc, this.getSelectedOptionValue()); break;
		}
	}		
}

UIOption.prototype.bindEvents = function()
{
	var me = this;
	var nOptions = me.optionValue.length;
	for (var i=0; i<nOptions; i++)
	{
		var btnID = this.id +"_opt" +i;
		var node = document.getElementById(btnID);
		
		node.addEventListener("mousedown", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
			{
				me.clickStartTimeMSec[i] = -1;
				return;
			}
			me.clickPosX[i] = ev.clientX;
			me.clickPosY[i] = ev.clientY;
			me.clickStartTimeMSec[i] = ev.timeStamp;
		}, 
		true);
		
		node.addEventListener("mouseup", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
			{
				me.clickStartTimeMSec[i] = -1;
				return;
			}

			var timeElapsedMSec = parseInt(ev.timeStamp - me.clickStartTimeMSec[i]);
			var xdiff = parseInt(Math.abs(me.clickPosX[i] - ev.clientX));
			var ydiff = parseInt(Math.abs(me.clickPosY[i] - ev.clientY));
			
			me.clickStartTimeMSec[i] = -1;
			if (timeElapsedMSec <650 && xdiff <60 &&ydiff<40)
			{
				me.selectOptionByIndex(this.getAttribute("data-optnum"));
				if (me.jsOnClick!="")
					eval(me.jsOnClick);
			}
		}, 
		true);		
	}	
}

UIOption.prototype.getNumOptions = function() 				{ return this.optionValue.length; }
UIOption.prototype.getSelectedOptionIndex = function()		{ return this.selectedOption; }
UIOption.prototype.selectOptionByIndex = function(i)
{
	i = parseInt(i);
	//console.log ("UIOption => selectOptionByIndex[" +i +"], currentSelected[" +this.selectedOption +"]");
	
	if (i<0) i=0;
	else if (i>=this.optionValue.length) i= this.optionValue.length-1;
	
	if (i == this.selectedOption)
		return;
	
	if (this.selectedOption >= 0)
	{
		var btnID = this.id +"_opt" +this.selectedOption;
		document.getElementById(btnID).classList.remove("UIlit"); 
	}
	
	this.selectedOption = i;
	var btnID = this.id +"_opt" +this.selectedOption;
	document.getElementById(btnID).classList.add("UIlit"); 
	
	//console.log ("UIOption => current value[" +this.getSelectedOptionValue() +"]");
}

UIOption.prototype.getSelectedOptionValue = function()
{
	if (this.selectedOption<0) return "";
	return this.optionValue[this.selectedOption];
}
UIOption.prototype.selectOptionByValue = function (v)
{
	//console.log ("UIOption => selectOptionByValue[" +v +"]");
	for (var i=0; i<this.optionValue.length; i++)
	{
		if (v == this.optionValue[i])
		{
			this.selectOptionByIndex(i);
			return;
		}
	}
	this.selectOptionByIndex(-1);
}