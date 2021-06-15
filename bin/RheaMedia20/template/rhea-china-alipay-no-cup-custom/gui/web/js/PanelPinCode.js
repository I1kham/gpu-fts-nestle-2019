function PanelPinCode (fatherDivID)
{
	this.currentPin = "";
	this.callback = null;
	this.isPinCodeSet = -1;
	var me = this;
	
	var style_pagePinCode = "display:none; position:absolute; top:0; left:0; background-color:rgba(0,0,0,0.8); width:1024px; height:600px; z-index:999; color:#fff; font-size:1.0em; text-align:center;";
	var style_pagePinCodeButton = "border:solid 1px #666; width:130px; height:20px; padding:0.7em 0 1.0em 0; text-align:center; font-size:2.0em; margin:5px; display:inline-block; background-color:#666; border-radius: 5px;";
	var style_pagePinCodeDisplay = "border:solid 4px #666; border-radius:5px 5px 0 0; text-align:center; color:#fff; width:100%; padding:0.5em 0 0.5em 0; background-color:#000; min-height:30px;";
	
	
	var divContainer = document.createElement("div");
		divContainer.id = "divPanelPinCode";
		divContainer.style = style_pagePinCode;

		var d1 = document.createElement("div");
			d1.style = "margin:10px auto 0 auto; display:inline-block; width:500px;";
			divContainer.appendChild(d1);	

			var dDisplay = document.createElement("div");
			dDisplay.id = "pagePinCodeDisplay";
			dDisplay.style = style_pagePinCodeDisplay;
			d1.appendChild(dDisplay);	

			for (var i=0; i<9; i++)
			{
				var d = document.createElement("div");
				d.style = style_pagePinCodeButton;
				d.innerHTML = (i+1);
				d.setAttribute ("data-num", i+1);
				d1.appendChild(d);
				
				d.addEventListener("mousedown", function (ev) {
					ev.stopPropagation();
					me.onBtnPressed (this.getAttribute("data-num"));
					//console.log (this.getAttribute("data-num"));
					//console.log (this);
					
				}, 
				true);
			}
			

	var fatherDiv = document.getElementById(fatherDivID);
	fatherDiv.appendChild(divContainer);	
}

PanelPinCode.prototype.show = function (callbackOK, callbackKO)
{
	//console.log ("this.isPinCodeSet:" +this.isPinCodeSet);
	//verifico se il pincode è impostato su CPU
	if (this.isPinCodeSet == -1)
	{
		var me = this;
		rhea.ajax ("isQuickMenuPinCodeSet", "").then( function(result) {
			if ("Y" === result)
			{
				me.isPinCodeSet= 1;
				me.priv_doShow(callbackOK, callbackKO);
			}
			else
			{
				me.isPinCodeSet = 0;
				setTimeout (callbackOK, 10);
			}
			
		});
	}
	else if (this.isPinCodeSet == 1)
		this.priv_doShow(callbackOK, callbackKO);
	else
		callbackOK();
}

PanelPinCode.prototype.priv_doShow = function (callbackOK, callbackKO)
{
	this.currentPin = "";
	this.callbackOK = callbackOK;
	this.callbackKO = callbackKO;
	this.maxNumTry = 3;
	rheaSetElemHTML (rheaGetElemByID("pagePinCodeDisplay"), "");
	rheaShowElem (rheaGetElemByID("divPanelPinCode"));
}

PanelPinCode.prototype.onBtnPressed = function(btnPressed)
{
	this.currentPin += btnPressed.toString();
	//console.log ("curpin:" +this.currentPin);
	
	var d = rheaGetElemByID("pagePinCodeDisplay");
	if (this.currentPin.length >= 3)
	{
		rheaSetElemHTML (d, "***");
		
		//verifico la bontà del pin inserito
		var me = this;
		rhea.ajax ("checkQuickMenuPinCode", { "pin": this.currentPin }).then( function(result) {
			//console.log ("checkQuickMenuPinCode:" +result);
			if ("OK" === result)
			{
				rheaHideElem(rheaGetElemByID("divPanelPinCode"));
				setTimeout (me.callbackOK, 10);
			}
			else
			{
				me.currentPin = "";
				rheaSetElemHTML (d, "");
				me.maxNumTry--;
				if (me.maxNumTry <= 0)
				{
					rheaHideElem(rheaGetElemByID("divPanelPinCode"));
					setTimeout (me.callbackKO, 10);
				}
			}
		});

	}
	else
	{
		var maskedPin = "";
		for (var i=0; i<this.currentPin.length; i++)
			maskedPin += "*";
		rheaSetElemHTML (d, maskedPin);
	}
}