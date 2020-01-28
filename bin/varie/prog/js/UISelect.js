/***************************************************************
 * UISelect
 
 *	attributi consentiti:
 *
 *		data-option="value|caption|value|caption|....value|caption"
 *			opzionale data-selected="value"	=> indica l'opzione selezionata. Default "" == nessuna selezione
 *			opzionale data-da3="xxx"		=> locazione in memoria dalla quale leggere/scrivere il [data-selected]
 *			opzionale data-da3bit="4|8|16"	=> legge 4 o 8 bit dal da3, default 8
 *			opzionale data-onclick ="showPage('pageMaintenance')"
 */
function UISelect (parentID, childNum, node)
{
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}
	
	this.jsOnClick = UIUtils_getAttributeOrDefault(node, "data-onclick", "");

	//elenco opzioni
	this.optionValue = [];
	this.optionCaption = [];
	this.clickPosX = 0;
	this.clickPosY = 0;
	this.clickStartTimeMSec = -1;
	
	var e = node.getAttribute("data-option").split("|");
	var nOptions = 0;
	for (var i=0; i<e.length;)
	{
		this.optionValue[nOptions] = e[i++];
		this.optionCaption[nOptions] = e[i++];
		nOptions++;
	}
	
	//binding a da3
	this.da3offset = 0;
	this.da3bit = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3bit", "8"));
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	//inietto l'html
	this.priv_buildHTML(node);

	//opzione selezionata
	this.selectedOption = -1;
	if (this.da3Pos >= 0)
		this.loadFromDA3 (da3, this.da3Pos);
	else
		this.selectOptionByValue (UIUtils_getAttributeOrDefault(node, "data-selected", "-1"));
}

UISelect.prototype.priv_buildHTML = function(node)
{
	if (null == node)
		node = document.getElementById(this.id);
	
	var nOptions = this.optionValue.length;
	var cellSize = parseInt (100 / nOptions);
	var html = "<div style='position:relative'><div><table class='tselect'>";
	for (var i=0; i<nOptions; i++)
	{
		var rowID = this.id +"_row" +i;
		html += "<tr id='" +rowID +"' style='display:none' valign='middle'><td>" +this.optionCaption[i] +"</td></tr>";
	}
	html += "</table></div>"
			+"<div style='position:absolute; top:0; right:0' class='UISelectBtn'>...</div></div>";
	node.innerHTML = html;	
}

UISelect.prototype.changeOptions = function (str)
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
	this.priv_buildHTML(null);
	
	var n = this.selectedOption;
	this.selectedOption = -2;
	this.selectOptionByIndex(n);
}


UISelect.prototype.setDA3Offset = function(da3offset)			{ this.da3offset = da3offset;}
UISelect.prototype.loadFromDA3 = function(da3)					
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
UISelect.prototype.saveToDA3 = function(da3)					
{ 
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

UISelect.prototype.bindEvents = function()
{
	var me = this;

	var node = document.getElementById(me.id);
	node.addEventListener("mousedown", function (ev)
	{
		if (this.classList.contains("UIdisabled"))
		{
			me.clickStartTimeMSec = -1;
			return;
		}
		me.clickPosX = ev.clientX;
		me.clickPosY = ev.clientY;
		me.clickStartTimeMSec = ev.timeStamp;
	}, 
	true);
		
	node.addEventListener("mouseup", function (ev)
	{
		if (this.classList.contains("UIdisabled"))
		{
			me.clickStartTimeMSec = -1;
			return;
		}

		var timeElapsedMSec = parseInt(ev.timeStamp - me.clickStartTimeMSec);
		var xdiff = parseInt(Math.abs(me.clickPosX - ev.clientX));
		var ydiff = parseInt(Math.abs(me.clickPosY - ev.clientY));
		
		me.clickStartTimeMSec = -1;
		if (timeElapsedMSec <650 && xdiff <60 &&ydiff<40)
		{
			me.priv_openSelectPanel();
			if (me.jsOnClick!="")
				eval(me.jsOnClick);
		}
	}, 
	true);		
}

UISelect.prototype.getSelectedOptionIndex = function()		{ return this.selectedOption; }
UISelect.prototype.selectOptionByIndex = function(i)
{
	i = parseInt(i);
	//console.log ("UISelect => selectOptionByIndex[" +i +"], currentSelected[" +this.selectedOption +"]");
	
	if (i<0) i=0;
	else if (i>=this.optionValue.length) i= this.optionValue.length-1;
	
	if (i == this.selectedOption)
		return;
	
	this.selectedOption = i;
	var rowID = this.id +"_row" +i;
	for (var i=0; i<this.optionCaption.length; i++)
	{
		var rowID = this.id +"_row" +i;
		var elem = rheaGetElemByID(rowID);
		if (i == this.selectedOption)
			rheaSetDisplayMode(elem, "table-row");
		else
			rheaHideElem(elem);
	}
	
	//console.log ("UISelect => current value[" +this.getSelectedOptionValue() +"]");
}

UISelect.prototype.getSelectedOptionValue = function()
{
	if (this.selectedOption<0) return "";
	return this.optionValue[this.selectedOption];
}
UISelect.prototype.selectOptionByValue = function (v)
{
	//console.log ("UISelect => selectOptionByValue[" +v +"]");
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


var UISelect_cur = null;
function UISelect_hideFullPage()	{ rheaHideElem (rheaGetElemByID("UISelectFullPage")); }
function UISelect_onUserSelected(iSelected)
{
	UISelect_cur.selectOptionByIndex(iSelected);
	UISelect_cur = null;
	UISelect_hideFullPage();
}
	
UISelect.prototype.priv_openSelectPanel = function()
{
	UISelect_cur = this;
	var html = "<ul>";
	for (var i=0; i<this.optionValue.length; i++)	
	{
		var css="";
		if (i == this.selectedOption)
			css = "UIlit";
		html += "<li class='" +css +"' onmousedown='UISelect_onUserSelected(" +i +")'>" +this.optionCaption[i] +"</li>";
		
	}
	html += "</ul>";
	rheaSetDivHTMLByName("UISelectFullPage_cnt", html);
	
	rheaShowElem (rheaGetElemByID("UISelectFullPage"));
	
}