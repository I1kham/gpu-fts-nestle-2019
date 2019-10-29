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
	this.contentID = this.id +"_content"
	this.firstTimeShow = 1;
	this.visible = 0;
	this.childList = [];
	elem.querySelector("div.UIWindowContent").setAttribute("id", this.contentID);
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

	var wrapperH = theWrapper.offsetHeight;
	var contentH = elemContent.offsetHeight;
	console.log ("winID[" +this.id +"], wrapperH[" +wrapperH +"], contentH[" +contentH +"]");

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
 * UIButtonattributi
 
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
		this.id = parentID +"_btn" +childNum
		node.setAttribute("id", this.id);
	}
	
	this.jsOnClick = node.getAttribute("data-onclick");

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
		node.addEventListener("mouseup", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
				return;
			eval(me.jsOnClick);
		}, 
		true);
		
	}
}