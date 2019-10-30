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



	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;
	//console.log ("winID[" +this.id +"], wrapperH[" +wrapperH +"], contentH[" +contentH +"]");

	if (contentH > wrapperH)
	{
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
			info.mouse_pressed = 1;
			info.mouse_y = ev.clientY;
		}, true);


		elemContent.addEventListener('mouseup', function (ev) 
		{
			info.mouse_pressed = 0;
		}, true);

		elemContent.addEventListener('mousemove', function (ev) 
		{
			if (!info.mouse_pressed)
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
		this.id = parentID +"_btn" +childNum;
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
		this.id = parentID +"_btn" +childNum;
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
	console.log ("selectOption[" +i +"], currentSelected[" +this.selectedOption +"]");
	
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
