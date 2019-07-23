var RHEADEBUG_MIN_H = 20;
var RHEADEBUG_MAX_H = 150;

function rheaDebug_showWindow()
{
	var d = rheaDoesElemExistsByID("rheaDebugWin");
	if (undefined === d)
	{
		if (undefined === rhea.Session_getValue("debug_console", ""))
			rhea.Session_setValue("debug_console", "");
		
		var divMainWrapper = rheaGetElemByID("mainWrapper");
			d = document.createElement("div");
			d.id = "rheaDebugWin";
			d.setAttribute ("style", "position:absolute; top:5px; left:85px; width:80%; height:" +RHEADEBUG_MAX_H +"px; background-color:rgba(255,255,255,0.9); color:#000; border:solid 2px #000; padding:4px; z-index:10");
		divMainWrapper.insertBefore(d, divMainWrapper.firstChild);
		
		
		var html = "<div id='rheaDebugWin_console' style='font-family:Courier,monospace; width:100%; height:100%; overflow:hidden; overflow-y:auto; font-size:16px;'>";
			html+=     rhea.Session_getValue("debug_console");
			html+= "</div>";
			html+= "<div style='position:absolute; top:0; right:20px; background-color:#f00; color:#fff; padding:2px; font-weight:bold' onclick='rheaDebug_minMaxWinDebug()'>X</div>";
		
		d = rheaDoesElemExistsByID("rheaDebugWin");
		rheaSetElemHTML(d, html);
	}
	
	//rheaShowElem(d);	
	//
}

function rheaDebug_addZero(i)
{
	if (i < 10)
    	i = "0" + i;
  return i;
}

function rheaDebug_addText (s)
{
	var today = new Date();
	var time = rheaDebug_addZero(today.getHours()) + ":" + rheaDebug_addZero(today.getMinutes()) + ":" + rheaDebug_addZero(today.getSeconds());
	
	var text = rhea.Session_getValue("debug_console");
	text = time +">  " +s +"<br>" + text;
	
	if (text.length > 10240)
		text = text.substr(0, 5000);	
	
	rhea.Session_setValue("debug_console", text);
	
	var d = rheaGetElemByID("rheaDebugWin_console");
	rheaSetElemHTML(d, text);
}

function rheaDebug_minMaxWinDebug()
{
	var d = rheaGetElemByID("rheaDebugWin");
	var h = rheaGetElemHeight(d);
	console.log (h);
	if (h <= RHEADEBUG_MAX_H)
		rheaSetElemHeight(d, RHEADEBUG_MAX_H);
	else
		rheaSetElemHeight(d, RHEADEBUG_MIN_H);
}