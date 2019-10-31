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
	//console.log ("winID[" +this.id +"], wrapperH[" +wrapperH +"], contentH[" +contentH +"]");

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

	var caption = node.getAttribute("data-caption");
	node.innerHTML = "<p>" +caption +"</p>";
	if (status != "" && status != "enabled")
		node.classList.add(status); 
}

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
			//console.log ("mdown => id[" +me.id +"], mx[" +me.clickPosX +"], my[" +me.clickPosY +"]");
		}, 
		true);
		
		node.addEventListener("mouseup", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
			{
				me.clickStartTimeMSec = -1;
				return;
			}

			//console.log ("mup => id[" +me.id +"], elapsed_msec[" +timeElapsedMSec +"], xdiff[" +xdiff +"], ydiff[" +ydiff +"]");
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
 *		data-option1="MAINTENANCE" data-option2=".." .. data-optionN="..."
 *		data-selected="1"	=> indica l'opzione da 1 a N selezionata di default (0 == nessuna selezione)
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
	this.options = [];
	this.clickPosX = [];
	this.clickPosY = [];
	this.clickStartTimeMSec = [];
	
	var i=1;
	while(1)
	{
		var opt = node.getAttribute("data-option" +i);
		if (null == opt || opt == "")
			break;
		this.options.push(opt.toString());
		i++;
	}
	
	//opzione selezionata (0==nessuna)
	this.selectedOption = 0;
	var s = node.getAttribute("data-selected");
	if (null != s && s != "")
		this.selectedOption = parseInt(s);


	//inietto l'html
	var nOptions = this.options.length;
	var cellSize = parseInt (100 / nOptions);
	var html = "<table class='UIOption'><tr>";
	for (var i=0; i<nOptions; i++)
	{
		var btnID = this.id +"_opt" +(i+1);
		var css = "UIButton";
		if (this.selectedOption == (i+1))
			css += " UIlit";
		html += "<td width='" +cellSize +"%'><div id='" +btnID +"' class='" +css +"' data-optnum='" +(i+1) +"'><p>" +this.options[i] +"</p></div></td>";
	}
	html += "</tr></table>";
	node.innerHTML = html;
}

UIOption.prototype.bindEvents = function()
{
	var me = this;
	var nOptions = me.options.length;
	for (var i=0; i<nOptions; i++)
	{
		var btnID = this.id +"_opt" +(i+1);
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
				me.selectOption(this.getAttribute("data-optnum"));
		}, 
		true);		
	}	
}

UIOption.prototype.getSelectedOption = function()		{ return this.selectOption; }

UIOption.prototype.selectOption = function(i)
{
	i = parseInt(i);
	//console.log ("selectOption[" +i +"], currentSelected[" +this.selectedOption +"]");
	
	if (i<1) i=0;
	else if (i>this.options.length) i= this.options.length;
	
	if (i == this.selectedOption)
		return;
	
	if (this.selectedOption > 0)
	{
		var btnID = this.id +"_opt" +this.selectedOption;
		document.getElementById(btnID).classList.remove("UIlit"); 
	}
	
	this.selectedOption = i;
	var btnID = this.id +"_opt" +this.selectedOption;
	document.getElementById(btnID).classList.add("UIlit"); 
}




/***************************************************************
 * UINumber
 
 *	attributi consentiti:
 *
 *		data-numfigures="2"			=> numero totale di cifre da visualizzare
 *		opzionale data-value="7"	=> valore da visualizzare (0 se non indicato)
 */
var UINUMBER_TOP_OFFSET = 15-400;
var UINUMBER_NUM_HEIGHT = 56;
function UINumber (parentID, childNum, node, parentObj)
{
	this.parentObj = parentObj;
	
	this.id = node.getAttribute("id");
	if (this.id==null || this.id=="")
	{
		this.id = parentID +"_child" +childNum;
		node.setAttribute("id", this.id);
	}

	this.numCifre = parseInt(node.getAttribute("data-numfigures"));
	if (this.numCifre < 1) this.numCifre = 1;
	else if (this.numCifre > 12) this.numCifre = 12;
	
	this.value = node.getAttribute("data-value");
	if (null == this.value || this.value == "")
		this.value = 0;
	else
		this.value = parseInt(this.value);

	this.mouseYStart=[];
	this.stripStartY=[];

	//genero l'HTML
	var html = "";
	for (var i=0; i<this.numCifre; i++)
	{
		this.mouseYStart[i] = 0;
		this.stripStartY[i] = 0;
		html += this.priv_getHTMLForAFigure(i);
	}
	node.innerHTML = html;
	
	this.setValue(this.value);
}

UINumber.prototype.priv_getHTMLForAFigure = function(i)
{
	var idStrip = this.id +"_fig" +i;
	var idContainer = idStrip+"_cnt";
	var idBorder = idStrip+"_brd";	
	var html = "<div class='UINumberContainer' id='" +idContainer +"'><div class='UINumberStrip' id='" +idStrip +"'><p>0</p><p>1</p><p>2</p><p>3</p><p>4</p><p>5</p><p>6</p><p>7</p><p>8</p><p>9</p></div><div class='UINumberMask'>&nbsp;</div><div class='UINumberBorder' id='" +idBorder +"'>&nbsp;</div></div>";
	return html;
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
		//console.log("down => y[" +me.mouseYStart[iCifra] +"], stripStartY[" +me.stripStartY[iCifra] +"]");
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
		
		if (relY>4) newY+=offsetY;
		if (relY>8)  newY+=offsetY;
		
		if (newY>UINUMBER_TOP_OFFSET) newY=UINUMBER_TOP_OFFSET;
		else 
		{
			var limit = UINUMBER_TOP_OFFSET - UINUMBER_NUM_HEIGHT*9;
			if (newY<limit) newY=limit;
		}
		dStrip.style.top = newY +"px";
		
		me.parentObj.enablePageScroll(0);
		//console.log("strip move => my[" +offsetY +"], relY[" +relY +"]");
	}, 
	true);		

	node.addEventListener("mouseleave", function (ev)
	{
		//console.log("strip leave");
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
	
	var dParent = document.getElementById(this.parentObj.id);
	dParent.zindex=1;
	dParent.style.overflow="hidden";
	this.parentObj.enablePageScroll(1);
	//console.log("strip up, y[" +curY +"], whichNum[" +whichNum +"]");
}

UINumber.prototype.getValue = function()			{ return this.value; } 

UINumber.prototype.setValue = function(v)
{
	//console.log("setValue[" +v +"]");
	this.value = parseInt(v);
	if (this.value==null || this.value=="")
		this.value = 0;
	
	var s = this.value.toString();
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
	}
}