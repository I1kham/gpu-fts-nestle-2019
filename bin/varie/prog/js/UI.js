function UIUtils_getAttributeOrDefault(node, attributeName, defValue)
{
	var r = node.getAttribute(attributeName);
	if (r==null || r=="") return defValue;
	return r;	
}

/********************************************************
 * UI
 *
 */
function UI(elemID)
{
	this.winList = [];
	
	var container = document.querySelector("#" +elemID);
	nodeList = container.querySelectorAll("div.UIWindow");	
	for (var i = 0; i < nodeList.length; i++)
		this.winList[i] = new UIWindow(nodeList[i]);
}

UI.prototype.priv_findByID = function(windowID)
{
	var n = this.winList.length;
	for (var i=0; i<n; i++)
	{
		if (this.winList[i].id == windowID)
			return i;
	}
	return -1;
}

UI.prototype.getWindowByID = function(windowID)
{
	var i = this.priv_findByID(windowID);
	if (i >= 0)
		return this.winList[i];
	return null;
}


UI.prototype.show = function(windowID)
{
	var i = this.priv_findByID(windowID);
	if (i >= 0)
		this.winList[i].show();			
}

UI.prototype.hide = function(windowID)
{
	var i = this.priv_findByID(windowID);
	if (i >= 0)
		this.winList[i].hide();			
}

UI.prototype.hideAllExcept = function(windowID)
{
	var n = this.winList.length;
	for (var i=0; i<n; i++)
	{
		if (this.winList[i].id == windowID)
			this.winList[i].show();
		else
			this.winList[i].hide();
	}
}

//crea un UIButton anche se questo non è all'interno di un UIPanel
UI.prototype.setupStandaloneButton = function(elemID)
{
	var o = new UIButton ("", 0, document.getElementById(elemID));
	o.bindEvents();
	return o;
}

//crea un UINumber anche se questo non è all'interno di un UIPanel
UI.prototype.setupStandaloneNumber = function(elemID)
{
	var o = new UINumber ("", 0, document.getElementById(elemID));
	o.bindEvents();
	return o;
}

//********************************************************
function UIWindowScrollable (elem, contentH, wrapperH) 
{ 
	this.elem = elem; 
	this.mouse_pressed =0; 
	this.mouse_y = 0; 
	this.scroll_miny = -(contentH - wrapperH);
	this.scroll_howMuch = 466; //(wrapperH / 2);
	this.scroll_tollerance_at_border = 5;
}



/********************************************************
 * UIWindow
 *
 */
function UIWindow(elem)
{
	this.id = elem.getAttribute("id");
	this.contentID = 
	this.firstTimeShow = 1;
	this.visible = 0;
	this.allowScroll = 1;
	this.childList = [];
	
	//il div di contenuti deve avere un id. Glielo assegno io se non lo ha
	var e = elem.querySelector("div.UIWindowContent");
	this.contentID = e.getAttribute("id");
	if (this.contentID==null || this.contentID=="")
	{
		this.contentID = this.id +"_content";
		e.setAttribute("id", this.contentID);
	}
}

UIWindow.prototype.show = function()
{
	if (this.visible == 0)
	{
		var elem = document.getElementById(this.id);
		this.visible = 1;
		elem.style.display = "block";
		if (this.firstTimeShow == 1)
		{
			this.firstTimeShow = 0;
			this.priv_setupAtFirstShow();
		}
	}
}

UIWindow.prototype.hide = function()
{
	if (this.visible == 1)
	{
		var elem = document.getElementById(this.id);
		this.visible = 0;
		elem.style.display = "none";
	}	
}

UIWindow.prototype.loadFromDA3 = function(da3, da3offset)
{
	for (var i = 0; i < this.childList.length; i++)
	{
		this.childList[i].setDA3Offset (da3offset);
		this.childList[i].loadFromDA3(da3);
	}
}

UIWindow.prototype.saveToDA3 = function(da3)
{
	for (var i = 0; i < this.childList.length; i++)
		this.childList[i].saveToDA3(da3);	
}

UIWindow.prototype.priv_findByID = function(childID)
{
	var n = this.childList.length;
	for (var i=0; i<n; i++)
	{
		if (this.childList[i].id == childID)
			return i;
	}
	return -1;
}

UIWindow.prototype.getChildByID = function(childID)
{
	var i = this.priv_findByID(childID);
	if (i >= 0)
		return this.childList[i];
	return null;
}


UIWindow.prototype.enablePageScroll = function (b) { this.allowScroll=b; }

UIWindow.prototype.priv_setupAtFirstShow = function()
{
	theWrapper = document.getElementById(this.id);
	elemContent = document.getElementById(this.contentID);

	//spawno i componenti UI contenuti nel content
	var childNum = 0;
	var nodeList = elemContent.querySelectorAll(":scope div.UIButton");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIButton(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	var nodeList = elemContent.querySelectorAll(":scope div.UIButtonSel");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIButtonSel(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	nodeList = elemContent.querySelectorAll(":scope div.UIOption");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIOption(this.id, childNum, nodeList[i]);
		childNum++;
	}
	nodeList = elemContent.querySelectorAll(":scope div.UIOptionSmall");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIOption(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	nodeList = elemContent.querySelectorAll(":scope div.UINumber");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UINumber(this.id, childNum, nodeList[i]);
		childNum++;
	}

	nodeList = elemContent.querySelectorAll(":scope div.UITime24");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UITime24(this.id, childNum, nodeList[i], this);
		childNum++;
	}


	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;
	//console.log ("UIWindow => winID[" +this.id +"], wrapperH[" +wrapperH +"], contentH[" +contentH +"]");

	if (contentH > wrapperH+10)
	{
		var me = this;
		
		//è necessario impostare lo scrolling
		var divIDArrowUp = this.id +"_arrowUp";
		var divIDArrowDown = this.id +"_arrowDown";
	
		//aggiungo freccia su/giu al wrapper
		var arrowDownY = wrapperH - 40;
		var html = "<div id='" +divIDArrowUp   +"' class='bigScrollArrowUp'    style='left:438px; top:0; display:none'><center><img draggable='false' src='img/big-arrow-up.png' height='30'></center></div>";
		    html += "<div id='" +divIDArrowDown +"' class='bigScrollArrowDown' style='left:438px; top:" +arrowDownY +"px; display:none'><center><img draggable='false' style='margin-top:11px' src='img/big-arrow-down.png' height='30'></center></div>";
		
		theWrapper.innerHTML += html;
		theWrapper.querySelector("#" +divIDArrowDown).style.display = "block";
	
		elemContent = document.getElementById(this.contentID);
		var info = new UIWindowScrollable(elemContent, contentH, wrapperH);
		elemContent.addEventListener('mousedown', function (ev)
		{
			//console.log ("UIWindow::mousedown");
			info.mouse_pressed = 1;
			info.mouse_y = ev.clientY;
		}, true);


		elemContent.addEventListener('mouseup', function (ev) 
		{
			//console.log ("UIWindow::mouseup");
			info.mouse_pressed = 0;
		}, true);

		elemContent.addEventListener('mousemove', function (ev) 
		{
			//console.log ("UIWindow::mousemove");
			if (!info.mouse_pressed || me.allowScroll==0)
				return;
				
			var y = ev.clientY;
			var offset = y - info.mouse_y;
			if (Math.abs(offset) < 10)
				return;
			info.mouse_y = y;
			
			var top = oldTop = rheaGetElemTop(info.elem);
			top += offset;
			if (top >= 0)
				top = 0;
			if (top < info.scroll_miny)
				top = info.scroll_miny;
			
			rheaSetElemTop(info.elem, top);
			//console.log ("move::rheaSetElemTop[" +top +"], offset[" +offset +"]");
			
			if (top < -info.scroll_tollerance_at_border)
				rheaShowElem(rheaGetElemByID(divIDArrowUp));
			else
				rheaHideElem(rheaGetElemByID(divIDArrowUp));

			if (top < (info.scroll_miny + info.scroll_tollerance_at_border))
				rheaHideElem(rheaGetElemByID(divIDArrowDown));
			else
				rheaShowElem(rheaGetElemByID(divIDArrowDown));
			
		}, true);
	
		//bindo onclick della freccia giù
		var theDiv = rheaGetElemByID(divIDArrowDown);
		theDiv.addEventListener('click', function (ev) 
		{
			var curY = rheaGetElemTop (info.elem);
			curY -= info.scroll_howMuch;
			if (curY <= (info.scroll_miny + info.scroll_tollerance_at_border))
			{
				curY = info.scroll_miny;
				rheaHideElem(rheaGetElemByID(divIDArrowDown));
			}
			
			rheaShowElem(rheaGetElemByID(divIDArrowUp));
			//rheaSmoothScrollElemTop(info.elem, curY, 300);
			rheaSetElemTop(info.elem, curY);
		}, true);
	
		//bindo onclick della freccia su
		theDiv = rheaGetElemByID(divIDArrowUp);
		theDiv.addEventListener('click', function (ev) 
		{
			var curY = rheaGetElemTop (info.elem);
			curY += info.scroll_howMuch;
			if (curY >= -info.scroll_tollerance_at_border)
			{
				rheaHideElem(rheaGetElemByID(divIDArrowUp));
				curY = 0;
			}
			
			rheaShowElem(rheaGetElemByID(divIDArrowDown));
			//rheaSmoothScrollElemTop(info.elem, curY, 300);
			rheaSetElemTop(info.elem, curY, curY);
		}, true);	
	}
	
	//setup degli eventi dei figli
	var n = this.childList.length;
	for (var i=0; i<n; i++)
		this.childList[i].bindEvents();

}



/***************************************************************
 * UIButton
 
 *	attributi consentiti:
 *
 *	opzionale 			data-caption="MAINTENANCE"
 *	opzionale 			data-onclick="showPage('pageMaintenance')"
 */
function UIButton (parentID, childNum, node)
{
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}
	
	this.jsOnClick = node.getAttribute("data-onclick");
	this.clickPosX = -1;
	this.clickPosY = -1;
	this.clickStartTimeMSec = 0;
	this.visible = 1;

	var caption = node.getAttribute("data-caption");
	var captionID = this.id +"_caption";
	node.innerHTML = "<p id='" +captionID +"'>" +caption +"</p>";
	if (status != "" && status != "enabled")
		node.classList.add(status); 
}

UIButton.prototype.setDA3Offset = function(da3offset)	{}
UIButton.prototype.loadFromDA3 = function(da3)	{}
UIButton.prototype.saveToDA3 = function(da3)	{}
UIButton.prototype.setCaption = function (s)	{ rheaSetDivHTMLByName(this.id +"_caption", s); }	
UIButton.prototype.show = function()			{ if (this.visible == 0) { this.visible = 1; document.getElementById(this.id).style.display = "block"; } }
UIButton.prototype.hide = function()			{ if (this.visible == 1) { this.visible = 0; document.getElementById(this.id).style.display = "none"; }	}

UIButton.prototype.bindEvents = function()
{
	if (this.jsOnClick!="" && this.jsOnClick!=null)
	{
		var me = this;
		var node = document.getElementById(this.id);
		
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
			//console.log ("UIButton::mdown => id[" +me.id +"], mx[" +me.clickPosX +"], my[" +me.clickPosY +"]");
		}, 
		true);
		
		node.addEventListener("mouseup", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
			{
				me.clickStartTimeMSec = -1;
				return;
			}

			//console.log ("UIButton::mup => id[" +me.id +"], elapsed_msec[" +timeElapsedMSec +"], xdiff[" +xdiff +"], ydiff[" +ydiff +"]");
			var timeElapsedMSec = parseInt(ev.timeStamp - me.clickStartTimeMSec);
			var xdiff = parseInt(Math.abs(me.clickPosX - ev.clientX));
			var ydiff = parseInt(Math.abs(me.clickPosY - ev.clientY));
			
			me.clickStartTimeMSec = -1;
			if (timeElapsedMSec <650 && xdiff <60 &&ydiff<40)
				eval(me.jsOnClick);
		}, 
		true);		
	}
}




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
	this.da3offset = 0;
	this.da3bit = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3bit", "8"));
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	//inietto l'html
	var cellSize = parseInt (100 / nOptions);
	var html = "<table class='" +node.getAttribute("class") +"'><tr>";
	for (var i=0; i<nOptions; i++)
	{
		var btnID = this.id +"_opt" +i;
		var css = "UIButton";
		html += "<td width='" +cellSize +"%'><div id='" +btnID +"' class='" +css +"' data-optnum='" +i +"'><p>" +this.optionCaption[i] +"</p></div></td>";
	}
	html += "</tr></table>";
	node.innerHTML = html;

	//opzione selezionata
	this.selectedOption = -1;
	if (this.da3Pos >= 0)
		this.loadFromDA3 (da3, this.da3Pos);
	else
		this.selectOptionByValue (UIUtils_getAttributeOrDefault(node, "data-selected", "-1"));
}

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