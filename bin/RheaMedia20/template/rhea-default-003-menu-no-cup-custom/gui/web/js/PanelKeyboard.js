function PanelKeyboard (fatherDivID, numMenuGen)
{
	this.currentNumKeyboard = "";
	this.isNumKeyboardSet = -1;
	var me = this;
	numMenuGen=numMenuGen+1;
	
	var descr=rheaLang.LAB_SNACK_INSTRUCTION; 			//"please digit here the snack selection number and press 'OK' to confim it";
	var descr1=rheaLang.LAB_SNACK_SELECTION_NUMBER;		//"Selection Number";
	var style_pageNumKeyboard = "display:none; position:absolute;  background-color:rgba(0,0,0,0.8); z-index:999; color:#fff; font-size:1.0em; text-align:center;";
	var style_pageNumKeyboardButton = "border:solid 1px #666; width:115px; height:35px; padding:0.2em 0 0.4em 0; text-align:center; font-size:1.5em; margin:4px; display:inline-block; background-color:#666; border-radius: 1px;";
	var style_pageNumKeyboardDisplay = "border:solid 1px #666; border-radius:1px 1px 0 0; text-align:center; color:#fff; width:100%; padding:1.0em 0 0.5em 0; background-color:#000; min-height:30px;";
	var com_SnackValInit = parseInt(MGEN_getKeyboard_SnackValInit(parseInt(numMenuGen)));
	var com_daSel=parseInt(MGEN_getKeyboard_daSel(parseInt(numMenuGen)));	
	var com_aSel=parseInt(MGEN_getKeyboard_aSel(parseInt(numMenuGen)));	

	var html =   "<div id='divPanelKeyboard' style='" +style_pageNumKeyboard +"'>"
				+"	<div style='margin:40px 40px 40px 20px; display:inline-block; width:400px;'>"  //margin: top,right,bottom,left
				+"  <div id='pageNumKeyboardDisplay' style='" +style_pageNumKeyboardDisplay +"'>"
				+"</div>";
				
				
	for (var i=1; i<=12; i++)
	{
		var divID = "PanelKeyboardNum" +i;
		var com_vis=i;
		if (i==11) com_vis="0";
		if (i==12) com_vis="OK";
		if (i==10) com_vis="X";
		html += "<div id='" +divID +"' data-num='" +i +"' style='" +style_pageNumKeyboardButton +"'>" +com_vis +"</div>";
	}
	html +=	" <div class='KeyboardDisplay_descrSel' style='text-align:center;'>"+descr1+"</div>";
	html += "<div id='descrKeyboardDisplay' class='KeyboardDisplay_descr'>"+'<BR>'+descr+"</div>";
	html += "	</div>";


	var divContainer = document.createElement("div");
		divContainer.innerHTML = html;
	document.getElementById(fatherDivID).appendChild(divContainer);		

	for (var i=1; i<=12; i++)
	{
		var divID = "PanelKeyboardNum" +i;
		document.getElementById(divID).addEventListener("mousedown", function (ev) {
				ev.stopPropagation();
				var valSelez=me.onBtnPressed (this.getAttribute("data-num"));
				if ((valSelez!=undefined) && (valSelez!="") && (valSelez!="0") && (parseInt(valSelez) <= com_aSel) && (parseInt(valSelez) >= com_daSel))
				{
					//	var numMaxSel = MMI_getCount();
					//	if (parseInt(valSelez) <= numMaxSel)
					valSelez=parseInt(valSelez) + com_SnackValInit;
					startSelectionSnack(valSelez, numMenuGen);
				}
				//console.log (this.getAttribute("data-num"));
				//console.log (this);
					
		}, 
			true);
	}
}

PanelKeyboard.prototype.show = function ()
{
	//console.log ("this.isNumKeyboardSet:" +this.isNumKeyboardSet);
	if (this.isNumKeyboardSet == -1)
	{
		var me = this;
		me.isNumKeyboardSet= 1;
		me.priv_doShow();

	}
		if (this.isNumKeyboardSet == 1)
			this.priv_doShow();
}

PanelKeyboard.prototype.priv_doShow = function ()
{
	this.currentNumKeyboard = "";
	this.maxNumTry = 3;
	rheaSetElemHTML (rheaGetElemByID("pageNumKeyboardDisplay"), "");
	rheaShowElem (rheaGetElemByID("divPanelKeyboard"));
}

PanelKeyboard.prototype.onBtnPressed = function(btnPressed)
{
	if (btnPressed==11) btnPressed=0;
	if (btnPressed==12) 
	{
		return this.currentNumKeyboard;
	}
	if ((btnPressed!=10) && (btnPressed!=12))
		this.currentNumKeyboard += btnPressed.toString();
	
	var d = rheaGetElemByID("pageNumKeyboardDisplay");
	
	if (btnPressed==10) 
	{
		if (this.currentNumKeyboard.length > 1) 
		{		
			var resto = this.currentNumKeyboard;
			this.currentNumKeyboard=resto[0];
			resto = this.currentNumKeyboard;
			rheaSetElemHTML (d, resto);
			return;
		}
		else
		{
			this.currentNumKeyboard = "";
			var resto = this.currentNumKeyboard ;
			rheaSetElemHTML (d, resto);
			return;
		}	
	}
	
	if (this.currentNumKeyboard.length > 2)
	{
		var me = this;
	}
	else
	{
		var valVisual = this.currentNumKeyboard;
		rheaSetElemHTML (d, valVisual);
	}
}

function startSelectionSnack(valSelez, numMenuGen)
{	
		//deve partire la selezione direttamente da qui, senza passare per pageConfirm. 
		//E' necessario che la selezione prezzo a 0 (oppure siamo in freevend) --> perchè?????????
		console.log ("Starting sel num:" +valSelez);
		window.location = "pageSelInProgress.html?iconMenu=" + numMenuGen+"&selNum=" +valSelez +"&btnStop=" +0;
}