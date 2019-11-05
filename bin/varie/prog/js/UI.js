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
}

//********************************************************
function UIWindowScrollable (elem, contentH, wrapperH) 
{ 
	this.elem = elem; 
	this.mouse_pressed =0; 
	this.mouse_y = 0; 
	this.scroll_miny = -(contentH - wrapperH);
	this.scroll_howMuch = (wrapperH / 2);
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

UIWindow.prototype.loadFromDA3 = function(da3)
{
	for (var i = 0; i < this.childList.length; i++)
		this.childList[i].loadFromDA3(da3);	
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
		console.log (this.childList[i].id +" == " +childID);
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

	//spawno i componenti UI contenuiti nel content
	var childNum = 0;
	var nodeList = elemContent.querySelectorAll(":scope div.UIButton");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIButton(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	nodeList = elemContent.querySelectorAll(":scope div.UIOption");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UIOption(this.id, childNum, nodeList[i]);
		childNum++;
	}
	
	nodeList = elemContent.querySelectorAll(":scope div.UINumber");
	for (var i = 0; i < nodeList.length; i++)
	{
		this.childList[childNum] =  new UINumber(this.id, childNum, nodeList[i], this);
		childNum++;
	}



	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;
	//console.log ("UIWindow => winID[" +this.id +"], wrapperH[" +wrapperH +"], contentH[" +contentH +"]");

	if (contentH > wrapperH)
	{
		var me = this;
		
		//è necessario impostare lo scrolling
		var divIDArrowUp = this.id +"_arrowUp";
		var divIDArrowDown = this.id +"_arrowDown";
	
		//aggiungo freccia su/giu al wrapper
		var arrowDownY = wrapperH - 40;
		var html = "<div id='" +divIDArrowUp   +"' class='bigScrollArrowUp'   style='top:0; display:none'><center><img draggable='false' src='img/big-arrow-up.png' height='30'></center></div>";
		   html += "<div id='" +divIDArrowDown +"' class='bigScrollArrowDown' style='top:" +arrowDownY +"px; display:none'><center><img draggable='false' style='margin-top:11px' src='img/big-arrow-down.png' height='30'></center></div>";
		
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
			info.mouse_y = y;
			
			var top = rheaGetElemTop(info.elem);
			top += offset;
			if (top >= 0)
				top = 0;
			if (top < info.scroll_miny)
				top = info.scroll_miny;
			rheaSetElemTop(info.elem, top);
			
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
			rheaSmoothScrollElemTop(info.elem, curY, 300);
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
			rheaSmoothScrollElemTop(info.elem, curY, 300);
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
 */
function UIOption (parentID, childNum, node)
{
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}

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
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	//inietto l'html
	var cellSize = parseInt (100 / nOptions);
	var html = "<table class='UIOption'><tr>";
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
		this.selectOptionByValue (da3.read16(this.da3Pos));
	else
		this.selectOptionByValue (UIUtils_getAttributeOrDefault(node, "data-selected", "-1"));
}

UIOption.prototype.loadFromDA3 = function(da3)		{ if (this.da3Pos >= 0) this.selectOptionByValue (da3.read16(this.da3Pos)); }
UIOption.prototype.saveToDA3 = function(da3)		{ if (this.da3Pos >= 0) da3.write16(this.da3Pos, this.getSelectedOptionValue()); }

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
				me.selectOptionByIndex(this.getAttribute("data-optnum"));
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

/***************************************************************
 * UINumber
 
 *	attributi consentiti:
 *
 *		data-numfigures="2"			=> numero totale di cifre da visualizzare
 *			opzionale data-value="7"	=> valore da visualizzare (0 se non indicato)
 *			opzionale data-da3="xxx"	=> locazione in memoria dalla quale leggere/scrivere il numero
 *			opzionale data-min			=> default = 0
 *			opzionale data-max			=> default = 999999999
 *			opzionale data-decimal		=> numero di cifre decimali dopo il "." 
											ATTENZIONE che UINumber ritorna sempre e cmq un numero intero, il decimale è solo un fatto estetico
 */
var UINUMBER_TOP_OFFSET = 15-400;
var UINUMBER_NUM_HEIGHT = 56;
function UINumber (parentID, childNum, node, parentObj)
{
	this.parentObj = parentObj;
	this.valueMin = parseInt(UIUtils_getAttributeOrDefault(node, "data-min", "0"));
	this.valueMax = parseInt(UIUtils_getAttributeOrDefault(node, "data-max", "999999999;"));
	
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
	this.da3Pos = parseInt(UIUtils_getAttributeOrDefault(node, "data-da3", "-1"));
	
	var value = 0;
	if (this.da3Pos >= 0)
		value = parseInt(da3.read16(this.da3Pos));
	else
		value = parseInt(UIUtils_getAttributeOrDefault(node, "data-value", "0"));

	this.mouseYStart=[];
	this.stripStartY=[];

	var numDecimalAfterPoint = parseInt(UIUtils_getAttributeOrDefault(node, "data-decimal", "0"));
	
	//genero l'HTML
	var html = "";
	for (var i=0; i<this.numCifre; i++)
	{
		this.mouseYStart[i] = 0;
		this.stripStartY[i] = 0;
		html += this.priv_getHTMLForAFigure(i);
		
		if (numDecimalAfterPoint>0 && i == this.numCifre - numDecimalAfterPoint -1)
			//html += "<div style='display:inline-block; font-size:3.0em'>.</div>";
		html += "<div class='UINumberContainer' style='width:20px; font-size:3.0em; line-height:120px'>.</div>";
	}
	node.innerHTML = html;
	
	this.setValue(value);
}

UINumber.prototype.priv_getHTMLForAFigure = function(i)
{
	var idStrip = this.id +"_fig" +i;
	var idContainer = idStrip+"_cnt";
	var idBorder = idStrip+"_brd";	
	var html = "<div class='UINumberContainer' id='" +idContainer +"'><div class='UINumberStrip' id='" +idStrip +"'><p>0</p><p>1</p><p>2</p><p>3</p><p>4</p><p>5</p><p>6</p><p>7</p><p>8</p><p>9</p></div><div class='UINumberMask'>&nbsp;</div><div class='UINumberBorder' id='" +idBorder +"'>&nbsp;</div></div>";
	return html;
}

UINumber.prototype.loadFromDA3 = function(da3)		{ if (this.da3Pos >= 0) this.setValue (da3.read16(this.da3Pos)); }
UINumber.prototype.saveToDA3 = function(da3)		{ if (this.da3Pos >= 0) da3.write16(this.da3Pos, this.getValue()); }


UINumber.prototype.bindEvents = function()
{
	for (var i=0; i<this.numCifre; i++)
		this.priv_bindEvents(i);
}

UINumber.prototype.priv_bindEvents = function(iCifra)
{
	var me = this;
	var idStrip = this.id +"_fig" +iCifra;
	var idBorder = idStrip+"_brd";	
	var idContainer = idStrip+"_cnt";

	var node = document.getElementById(idBorder);
	node.addEventListener("mousedown", function (ev)
	{
		var dParent = document.getElementById(me.parentObj.id);
		dParent.zindex=98;
		dParent.style.overflow="visible";
		
		var dContainer = document.getElementById(idContainer);
		dContainer.style.zIndex = 99;
		
		var dStrip = document.getElementById(idStrip);
		dStrip.style.zIndex = 100;
		me.stripStartY[iCifra] = parseInt(dStrip.offsetTop);
		dContainer.style.overflow="visible";
		me.mouseYStart[iCifra] = ev.clientY;
		
		me.parentObj.enablePageScroll(0);
		//console.log("UINumber::down => y[" +me.mouseYStart[iCifra] +"], stripStartY[" +me.stripStartY[iCifra] +"]");
	}, 
	true);
	
	node = document.getElementById(idStrip);
	node.addEventListener("mousemove", function (ev)
	{
		var my = ev.clientY;
		var offsetY = (my - me.mouseYStart[iCifra]);
		
		
		var dStrip = document.getElementById(idStrip);
		var newY = (me.stripStartY[iCifra] + offsetY);
		var relY = Math.abs(newY - parseInt(dStrip.style.top));
		
		//if (relY>4) newY+=offsetY;
		//if (relY>8)  newY+=offsetY;
		
		if (newY>UINUMBER_TOP_OFFSET) newY=UINUMBER_TOP_OFFSET;
		else 
		{
			var limit = UINUMBER_TOP_OFFSET - UINUMBER_NUM_HEIGHT*9;
			if (newY<limit) newY=limit;
		}
		dStrip.style.top = newY +"px";
		me.parentObj.enablePageScroll(0);
		//console.log("UINumber::strip move => my[" +offsetY +"], relY[" +relY +"]");
	}, 
	true);		

	node.addEventListener("mouseleave", function (ev)
	{
		//console.log("UINumber::strip leave");
		//me.priv_onMouseUp(ev, iCifra);
	}, 
	true);
	
	node.addEventListener("mouseup", function (ev)
	{
		me.priv_onMouseUp(ev, iCifra);
	}, 
	true);	
}

UINumber.prototype.priv_onMouseUp = function(ev, iCifra)
{
	var idStrip = this.id +"_fig" +iCifra;
	var idContainer = idStrip+"_cnt";	
	var dContainer = document.getElementById(idContainer);
	dContainer.style.zIndex = 1;
	
	var dStrip = document.getElementById(idStrip);
	dStrip.style.zIndex = 10;
	dContainer.style.overflow="hidden";

	var curY = parseInt(dStrip.offsetTop);
	var whichNum = -parseInt(Math.round((curY - UINUMBER_TOP_OFFSET) / UINUMBER_NUM_HEIGHT));
	dStrip.style.top = (UINUMBER_TOP_OFFSET - UINUMBER_NUM_HEIGHT*whichNum) +"px";
	dStrip.setAttribute("data-value", whichNum);
	
	var dParent = document.getElementById(this.parentObj.id);
	dParent.zindex=1;
	dParent.style.overflow="hidden";
	this.parentObj.enablePageScroll(1);
	//console.log("UINumber::priv_onMouseUp, y[" +curY +"], whichNum[" +whichNum +"]");
	
	//per assicurarmi di stare all'interno di valueMin/valueMax...
	this.setValue(this.getValue());
		
	console.log("UINumber::priv_onMouseUp => value=[" +this.getValue() +"]");
	
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
		dStrip.style.top = (UINUMBER_TOP_OFFSET - UINUMBER_NUM_HEIGHT*num) +"px";		
		dStrip.setAttribute("data-value", num);
	}
}