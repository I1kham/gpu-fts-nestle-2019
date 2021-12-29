function UIUtils_getAttributeOrDefault(node, attributeName, defValue)
{
	var r = node.getAttribute(attributeName);
	if (r==null || r=="") return defValue;
	return r;	
}

/**********************************************
 * 	2021-06-11: siamo passati da L R a Le Ri e da S M L a Sm Me La
 *	Invece di cambiarlo in tutto il codice, faccio un fn che converte la vecchia sintassi
 *	nella nuova al solo scopo di visualizzazione
 */
function UIUtils_arrangeSelectionOptionDescription2021(descr)
{		
	var ret = "";
	switch (descr.substr(0,1).toUpperCase())
	{
		case "L": ret += "Le"; break;
		case "R": ret += "Ri"; break;
		default: ret += "X"; break;
	}

	switch (descr.substr(1,1).toUpperCase())
	{
		case "L": ret += "Le"; break;
		case "R": ret += "Ri"; break;
		default: ret += "X"; break;
	}

	switch (descr.substr(2,1).toUpperCase())
	{
		case "S": ret += "Sm"; break;
		case "M": ret += "Me"; break;
		case "L": ret += "La"; break;
		default: ret += "X"; break;
	}
	
	return ret;
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

//crea un UIOption anche se questo non è all'interno di un UIPanel
UI.prototype.setupStandaloneUIOption = function(elemID)
{
	var o = new UIOption ("", 0, document.getElementById(elemID));
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


UIWindow.prototype.enablePageScroll = function (b) 
{ 
	this.allowScroll=b; 
}

UIWindow.prototype.priv_setupAtFirstShow = function()
{
	var theWrapper = document.getElementById(this.id);
	var elemContent = document.getElementById(this.contentID);

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
	
	nodeList = elemContent.querySelectorAll(":scope div.UISelect");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UISelect(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	//Setup dello scroll
	var me = this;
	
	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;
	
	//è necessario impostare lo scrolling
	me.divIDArrowUp = this.id +"_arrowUp";
	me.divIDArrowDown = this.id +"_arrowDown";

	//aggiungo freccia su/giu al wrapper
	var arrowDownY = wrapperH - 40;
	var html = "<div id='" +me.divIDArrowUp   +"' class='bigScrollArrowUp'    style='left:438px; top:0; display:none'><center><img draggable='false' src='img/big-arrow-up.png' height='30'></center></div>";
		html += "<div id='" +me.divIDArrowDown +"' class='bigScrollArrowDown' style='left:438px; top:" +arrowDownY +"px; display:none'><center><img draggable='false' style='margin-top:11px' src='img/big-arrow-down.png' height='30'></center></div>";
	
	theWrapper.innerHTML += html;
	theWrapper.querySelector("#" +me.divIDArrowDown).style.display = "block";


	elemContent = document.getElementById(this.contentID);
	me.info = new UIWindowScrollable(elemContent, contentH, wrapperH);
	elemContent.addEventListener('mousedown', function (ev)
	{
		//console.log ("UIWindow::mousedown");
		me.info.mouse_pressed = 1;
		me.info.mouse_y = ev.clientY;
	}, true);


	elemContent.addEventListener('mouseup', function (ev) 
	{
		//console.log ("UIWindow::mouseup");
		me.info.mouse_pressed = 0;
	}, true);

	elemContent.addEventListener('mousemove', function (ev) 
	{
		//console.log ("UIWindow::mousemove");
		if (!me.info.mouse_pressed || me.allowScroll==0)
			return;
			
		var y = ev.clientY;
		var offset = y - me.info.mouse_y;
		if (Math.abs(offset) < 10)
			return;
		me.info.mouse_y = y;
		
		var top = oldTop = rheaGetElemTop(me.info.elem);
		top += offset;
		if (top >= 0)
			top = 0;
		if (top < me.info.scroll_miny)
			top = me.info.scroll_miny;
		
		rheaSetElemTop(me.info.elem, top);
		//console.log ("move::rheaSetElemTop[" +top +"], offset[" +offset +"]");
		
		if (top < -me.info.scroll_tollerance_at_border)
			rheaShowElem(rheaGetElemByID(me.divIDArrowUp));
		else
			rheaHideElem(rheaGetElemByID(me.divIDArrowUp));

		if (top < (me.info.scroll_miny + me.info.scroll_tollerance_at_border))
			rheaHideElem(rheaGetElemByID(me.divIDArrowDown));
		else
			rheaShowElem(rheaGetElemByID(me.divIDArrowDown));
		
	}, true);

	//bindo onclick della freccia giù
	var theDiv = rheaGetElemByID(me.divIDArrowDown);
	theDiv.addEventListener('click', function (ev) 
	{
		var curY = rheaGetElemTop (me.info.elem);
		curY -= me.info.scroll_howMuch;
		if (curY <= (me.info.scroll_miny + me.info.scroll_tollerance_at_border))
		{
			curY = me.info.scroll_miny;
			rheaHideElem(rheaGetElemByID(me.divIDArrowDown));
		}
		
		rheaShowElem(rheaGetElemByID(me.divIDArrowUp));
		rheaSetElemTop(me.info.elem, curY);
	}, true);

	//bindo onclick della freccia su
	theDiv = rheaGetElemByID(me.divIDArrowUp);
	theDiv.addEventListener('click', function (ev) 
	{
		var curY = rheaGetElemTop (me.info.elem);
		curY += me.info.scroll_howMuch;
		if (curY >= -me.info.scroll_tollerance_at_border)
		{
			rheaHideElem(rheaGetElemByID(me.divIDArrowUp));
			curY = 0;
		}
		
		rheaShowElem(rheaGetElemByID(me.divIDArrowDown));
		//rheaSmoothScrollElemTop(info.elem, curY, 300);
		rheaSetElemTop(me.info.elem, curY, curY);
	}, true);	
		
	//setup degli eventi dei figli
	var n = this.childList.length;
	for (var i=0; i<n; i++)
		this.childList[i].bindEvents();	
	
	this.showHideScrollBar();
	
	
	console.log ("main=" +Keyboard.elements.main);
	if (Keyboard.elements.main === null)
		Keyboard.init ("EN");
}


UIWindow.prototype.showHideScrollBar = function()
{
	rheaSetElemTop(this.info.elem, 0,0);
	var theWrapper = document.getElementById(this.id);
	var elemContent = document.getElementById(this.contentID);

	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;

	if (contentH > wrapperH+10)
	{
		this.allowScroll = 1;
		rheaHideElem(rheaGetElemByID(this.divIDArrowUp));
		rheaShowElem(rheaGetElemByID(this.divIDArrowDown));
		this.info = new UIWindowScrollable(elemContent, contentH, wrapperH);
	}
	else
	{
		this.allowScroll = 0;
		rheaHideElem(rheaGetElemByID(this.divIDArrowUp));
		rheaHideElem(rheaGetElemByID(this.divIDArrowDown));
	}
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
UIButton.prototype.show = function()			{ if (this.visible == 0) { this.visible = 1; document.getElementById(this.id).style.display = "table"; } }
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
