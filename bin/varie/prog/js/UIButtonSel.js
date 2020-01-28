/***************************************************************
 * UIButtonSel
 
 *	attributi consentiti:
 *
 *	opzionale 			data-caption ="SELECTION 01"
 *	opzionale 			data-caption2="LATTE MACCHIATO"
 *	opzionale 			data-caption3="xxS"
 *	opzionale 			data-onclick="showPage('pageMaintenance')"
 */
function UIButtonSel (parentID, childNum, node)
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

	this.caption1 = UIUtils_getAttributeOrDefault(node, "data-caption", "");
	this.caption2 = UIUtils_getAttributeOrDefault(node, "data-caption2", "");
	this.caption3 = UIUtils_getAttributeOrDefault(node, "data-caption3", "xxx");
	if (this.caption3=="xxx")
		this.caption3="";
	
	shortImg="";
	if (this.caption3 != "")
	{
		shortImg = "<img src='img/sel" +this.caption3 +".png' draggable='false'>";
		
		//per ragioni storiche, lo short name è nel formato LLS, RLS... ora invece lo vogliamo vogliamo nel formato NNS, YNS..
		//Sostituisco i primi 2 char (che sono L o R o x) con N o Y
		var c1 = this.caption3.substr(0,1);
		var c2 = this.caption3.substr(1,1);
		var c3 = this.caption3.substr(2,1);
		if (c1=="L") c1="N";
		else if (c1=="R") c1="Y";
		if (c2=="L") c2="N";
		else if (c2=="R") c2="Y";
		this.caption3 = c1 +c2 +c3;		
	}
	
	var captionID = this.id +"_caption";
	node.innerHTML =  "<table width='100%'><tr valign='top'><td id='" +captionID +"'><b>" +this.caption1 +"</b><br>" +this.caption2 +"</td>"
					 +"<td width='50' align='center'>" +shortImg +"<br>" +this.caption3 +"</td></tr></table>";
	if (status != "" && status != "enabled")
		node.classList.add(status); 
}

UIButtonSel.prototype.setDA3Offset = function (da3offset) {}
UIButtonSel.prototype.loadFromDA3 = function(da3)	{}
UIButtonSel.prototype.saveToDA3 = function(da3)	{}
UIButtonSel.prototype.setCaption = function (s)	{ rheaSetDivHTMLByName(this.id +"_caption", s); }	
UIButtonSel.prototype.show = function()			{ if (this.visible == 0) { this.visible = 1; document.getElementById(this.id).style.display = "block"; } }
UIButtonSel.prototype.hide = function()			{ if (this.visible == 1) { this.visible = 0; document.getElementById(this.id).style.display = "none"; }	}

UIButtonSel.prototype.bindEvents = function()
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
			//console.log ("UIButtonSel::mdown => id[" +me.id +"], mx[" +me.clickPosX +"], my[" +me.clickPosY +"]");
		}, 
		true);
		
		node.addEventListener("mouseup", function (ev)
		{
			if (this.classList.contains("UIdisabled"))
			{
				me.clickStartTimeMSec = -1;
				return;
			}

			//console.log ("UIButtonSel::mup => id[" +me.id +"], elapsed_msec[" +timeElapsedMSec +"], xdiff[" +xdiff +"], ydiff[" +ydiff +"]");
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
