<?php
require_once "../REST/DBConn.php";
require_once "../REST/common.php";
$db = new DBConn();
$ses = sessionGetCurrent ($db);
sessionUpdateTimestamp ($db, $ses["sessionID"]);
?>
<html>
<head>
	<title>GUI</title>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
	<link href="style.css?dt=<?php echo time()?>" rel="stylesheet" type="text/css">
	<link href="styleOptionButtons.css?dt=<?php echo time()?>" rel="stylesheet" type="text/css">
</head>

<body onLoad="rheaBootstrap()">
<div id="mainWrapper" class="wrapper">
	<!--<div id="divCredit" class="header-credit">CREDIT -->
	<?php 
	//echo number_format ($ses["creditCurrent"], 2, ",", ".")
	?>
	<!--</div>-->
	<div class="header-sticky">
		<div id="divCPUMessage" style="display:none;"></div>	
	</div>
	
	<div id="beverageName" class="pageConfirm_beverageTitle pageConfirm_bigFont"></div>
	<div id="beveragePhoto" class="pageConfirm_beveragePhoto"></div>

	<div id="pleaseWait" style="display:none; text-align:center;"><img draggable='false' src="img/animationRound.gif"></div>
	<div id="beverageOptions">
		<table border="0" cellspacing="0" cellpadding="0" width="100%">
			<!-- selection descr (when no cup customization options are avail -->
			<tr valign="middle" id="rowSelDescription" style="display:none">
				<td align="center" id="tdSelDescription">
				</td>
			</tr>
			<!-- option A -->
			<tr valign="middle" id="divOption1" style="display:none">
				<td width="50%" align="center">
					<div id="divOption1-0" class="pageConfirm_iconOptionAL" onclick="onOption1Change(0)"><p>TEXT</p></div>
				</td>
				<td width="50%" align="center">
					<div id="divOption1-1" class="pageConfirm_iconOptionAR" onclick="onOption1Change(1)"><p>TEXT</p></div>
				</td>
			</tr>
			
			<tr valign="middle" id="divSeparator1" style="display:none">
				<td colspan="2" align="center"><div class="pageConfirm_separator"></div></td>
			</tr>
		
			<!-- option B -->					
			<tr valign="middle" id="divOption2" style="display:none">
				<td width="50%" align="center">
					<div id="divOption2-0" class="pageConfirm_iconOptionCL" onclick="onOption2Change(0)"><p>TEXT</p></div>
				</td>
				<td width="50%" align="center">
					<div id="divOption2-1" class="pageConfirm_iconOptionCR" onclick="onOption2Change(1)"><p>TEXT</p></div>
				</td>
			</tr>						
					
			<tr valign="middle" id="divSeparator2" style="display:none">
				<td colspan="2" align="center"><div class="pageConfirm_separator"></div></td>
			</tr>
			
			<!-- cup list -->
			<tr valign="middle" id="divCups" style="display:none">
				<td colspan="2" align="center">
					<div id="divCup0Container" class="pageConfirm_iconCupContainer pageConfirm_color1" onclick="onCupSelected(0)">
						<div id="divCup0" class="pageConfirm_iconCup cup0"></div>
						SMALL
					</div>
					<div id="divCup1Container" div class="pageConfirm_iconCupContainer pageConfirm_color1" onclick="onCupSelected(1)">
						<div id="divCup1" class="pageConfirm_iconCup cup1"></div>
						MEDIUM
					</div>
					<div id="divCup2Container" class="pageConfirm_iconCupContainer pageConfirm_color1" onclick="onCupSelected(2)">
						<div id="divCup2" class="pageConfirm_iconCup cup2"></div>
						LARGE
					</div>
				</td>
			</tr>
		</table>
		
		<div id="btnStart" class="btnStart pageConfirm_bigFont" onclick="onBtnStartSelection()"></div>
		<div id="btnStartNotAvail" class="btnStartNotAvail" style="display:none"></div>
	</div>
	
	<div id="divBtnHome" class="btnIndietro" onclick="gotoPageMenu()"><img draggable='false' src="img/btnHome.png"></div>
	<div id="divBtnSelectionInfo" class="pageConfirm_selectionInfo"  style="display:none" onclick="gotoSelectionInfo()"><img draggable='false' src="img/info.png"></div>
	<div class="clear">&nbsp;</div>
</div>

<script src="js/gjs-min.js"></script>
<script src="config/lang.js"></script>
<script src="config/mainMenuIcons.js"></script>
<script src="config/MMI.js"></script>
<script src="js/rheaBootstrap.js"></script>
<script language="javascript">
var timerGotoPageMenu = null;
var iconNum = 0;
var selThatWasStarted = 0;
var GRINDER = 0;
var CUP_SIZE = 0;
var SHOT_TYPE = 0;

function onRheaBootstrapFinished()	{ lang_loadLang(lang_getCurLangISOCode()).then (onAfterLangLoaded); }
function onAfterLangLoaded()		{ lang_loadJS ("cupAppearance").then ( function()	{ onAfterLangLoaded2(); }); }
function onAfterLangLoaded2()
{
	//in get ci deve essere il parametro che indica quale iconMenu è stata selezionata
	iconNum = parseInt(rheaGetURLParamOrDefault("iconMenu", 0));

	//carica le nutritional info
	lang_loadJS ("nutriInfo").then ( function()
	{
		if (rheaLang_nutriInfo[iconNum] != "")
			rheaShowElem(rheaGetElemByID("divBtnSelectionInfo"));
	});
		
	rhea.onEvent_cpuMessage = function(msg, importanceLevel)
	{
		var dCPU = rheaGetElemByID ("divCPUMessage");
		rheaSetElemHTML(dCPU, msg);
		rheaShowElem(dCPU);
	}
	
	rhea.onEvent_selectionReqStatus = function (status)
	{
		onSelectionStatusChanged(status, 0);
	}
		
	setupPageContent();
	rhea.requestGPUEvent(RHEA_EVENT_CPU_STATUS);	
	rhea.requestGPUEvent(RHEA_EVENT_CPU_MESSAGE);
	resetTimerGotoPageMenu();
}

function onSelectionStatusChanged(status, bSelectionStartedWithREST)
{
	switch (parseInt(status))
	{
		case 1: // wait for credit
			console.log ("sel waiting..");
			break;
			
		case 2:// selection started
			gotoPagePreparingSel(0, bSelectionStartedWithREST);
			break;
			
		case 5:// selection started, can use btnStop
			gotoPagePreparingSel(1, bSelectionStartedWithREST);
			break;

		default:				
		case 3: // selection aborted
			console.log ("sel aborted [" +status +"]");
			onSelectionKO();
			break;
	}	
}

function onSelectionKO()
{
	selThatWasStarted = 0;
	rheaShowElem(rheaGetElemByID("beverageOptions"));
	rheaShowElem(rheaGetElemByID("divBtnHome")); //btn back
	rheaShowElem(rheaGetElemByID("divBtnSelectionInfo")); //btn selection info
	rheaHideElem(rheaGetElemByID("pleaseWait"));	
}

function setupPageContent()
{
	var selNum = MMI_getLinkedSelectionNumber(iconNum);
	
	//nome bevanda
	var iconDisplayName = MMI_getDisplayName(iconNum);
	rheaSetDivHTMLByName("beverageName", iconDisplayName);

	//foto bevanda
	var image = "<img draggable='false' src='" +MMI_getImgForPageConfirm(iconNum) +"'>";
	rheaSetDivHTMLByName("beveragePhoto", image);
	
	//opzioni
	if (selNum > 0)
	{
		//non è una bevanda con customizzazioni
		//carica la descrizione della bevanda
		lang_loadJS ("mainMenuIconsInfo").then ( function()
		{
			var e = document.getElementById("tdSelDescription");
			e.innerHTML = rheaMainMenuIconsInfo[iconNum];
			rheaSetDisplayMode(rheaGetElemByID("rowSelDescription"), "table-row");			
		});			
	}
	else
	{
		//se arriviamo qui, vuol dire che stiamo gestendo una selezione customizzata
		var bShowOption1 = 0;
		var bShowOption2 = 0;
		if (MMI_hasDblGrinder(iconNum))
			bShowOption1=1;
		if (MMI_hasDoubleShot(iconNum))
			bShowOption2=1;
			
		//se c'è una sola tazza, non la vogliono vedere
		var bShowCupDiv = 0;
		if (MMI_canUseSmallCup(iconNum)) bShowCupDiv++;
		if (MMI_canUseMediumCup(iconNum)) bShowCupDiv++;
		if (MMI_canUseLargeCup(iconNum)) bShowCupDiv++;
		if (bShowCupDiv<2)
			bShowCupDiv=0;

		//a seconda delle opzioni disponibili, mostro o nascondo i div
		var n = 0;
		var cupOptionButtonText = rheaLang_cupAppearance[iconNum].split("§");
		if (bShowOption1)
		{
			n++;
			var e = document.getElementById("divOption1-0");
			e.classList = "pageConfirm_iconOption" +rheaLang_cupAppearance[iconNum].substr(0,1) +"L";
			e.innerHTML = "<p>" +cupOptionButtonText[1] +"</p>";
			
			e = document.getElementById("divOption1-1");
			e.classList = "pageConfirm_iconOption" +rheaLang_cupAppearance[iconNum].substr(0,1) +"R";
			e.innerHTML = "<p>" +cupOptionButtonText[2] +"</p>";
			rheaSetDisplayMode(rheaGetElemByID("divOption1"), "table-row");
		}
		
		if (bShowOption2)
		{
			n++;
			var e = document.getElementById("divOption2-0");
			e.classList = "pageConfirm_iconOption" +rheaLang_cupAppearance[iconNum].substr(1,1) +"L";
			e.innerHTML = "<p>" +cupOptionButtonText[3] +"</p>";
			
			e = document.getElementById("divOption2-1");
			e.classList = "pageConfirm_iconOption" +rheaLang_cupAppearance[iconNum].substr(1,1) +"R";
			e.innerHTML = "<p>" +cupOptionButtonText[4] +"</p>";
			
			rheaSetDisplayMode(rheaGetElemByID("divOption2"), "table-row");
		}

		if (bShowCupDiv)
		{
			n++;
			rheaSetDisplayMode(rheaGetElemByID("divCups"), "table-row");
		}

		if (n==3)
		{
			rheaSetDisplayMode(rheaGetElemByID("divSeparator1"), "table-row");
			rheaSetDisplayMode(rheaGetElemByID("divSeparator2"), "table-row");
		}
		else if (n==2)
		{
			if (bShowOption1)
				rheaSetDisplayMode(rheaGetElemByID("divSeparator1"), "table-row");
			else
				rheaSetDisplayMode(rheaGetElemByID("divSeparator2"), "table-row");
		}
	
		//disabilito eventuali cup-size non cliccabili
		if (!MMI_canUseSmallCup(iconNum))
			rheaHideElem(rheaGetElemByID("divCup0Container"), "disabled");
		if (!MMI_canUseMediumCup(iconNum))
			rheaHideElem(rheaGetElemByID("divCup1Container"), "disabled");
		if (!MMI_canUseLargeCup(iconNum))
			rheaHideElem(rheaGetElemByID("divCup2Container"), "disabled");
	}
	
	
	if (selNum > 0)
	{
		//accendo la cup-size di default
		if (MMI_canUseSmallCup(iconNum))
			onCupSelected(0);
		if (MMI_canUseMediumCup(iconNum))
			onCupSelected(1);			
		if (MMI_canUseLargeCup(iconNum))
			onCupSelected(2);			
	}
	else
	{	
		var defaultOption = MMI_getDefaultLinkedSelectionOptions(iconNum);
		
		onCupSelected (defaultOption.cupSize);
		onOption1Change (defaultOption.grinder);
		onOption2Change (defaultOption.shotType);
	}

}

function onCupSelected (whichOne)
{
	if (whichOne == 0 && !MMI_canUseSmallCup(iconNum))
		return;
	if (whichOne == 1 && !MMI_canUseMediumCup(iconNum))
		return;
	if (whichOne == 2 && !MMI_canUseLargeCup(iconNum))
		return;
		
		
		
	CUP_SIZE = whichOne;
	
	for (var i=0; i<3; i++)
	{
		var d = rheaGetElemByID("divCup" +i);
		rheaRemoveClassToElem(d,"selected");
		if (i == CUP_SIZE)
			rheaAddClassToElem(d,"selected");
	}
	
	var selNum = detectCurrentSelection();
	onSelectionChanged(selNum);
}


function onOption1Change (whichOne)
{
	GRINDER = whichOne;
	
	for (var i=0; i<2; i++)
	{
		var d = rheaGetElemByID("divOption1-" +i);
		rheaRemoveClassToElem(d,"selected");
		if (i == GRINDER)
			rheaAddClassToElem(d,"selected");
	}
	
	var selNum = detectCurrentSelection();
	onSelectionChanged(selNum);
}

function onOption2Change (whichOne)
{
	SHOT_TYPE = whichOne;
	
	for (var i=0; i<2; i++)
	{
		var d = rheaGetElemByID("divOption2-" +i);
		rheaRemoveClassToElem(d,"selected");
		if (i == SHOT_TYPE)
			rheaAddClassToElem(d,"selected");
	}
	
	var selNum = detectCurrentSelection();
	onSelectionChanged(selNum);
}

function detectCurrentSelection()
{
	var selNum = MMI_getLinkedSelectionNumber(iconNum);
	if (selNum == 0)
		selNum = MMI_getLinkedSelection (iconNum, GRINDER, CUP_SIZE, SHOT_TYPE);
	
	return selNum;
}

function gotoPagePreparingSel(bCanUseBtnStop, bSelectionStartedWithREST)
{
	var url = "pageSelInProgress.html";
	url += "?dt=<?php echo time();?>&iconMenu=" +iconNum +"&selNum=" +selThatWasStarted +"&btnStop=" +bCanUseBtnStop +"&rest=" +bSelectionStartedWithREST;
	window.location = url;
}

function gotoSelectionInfo()	{ window.location = "pageSelectionInfo.html?dt=<?php echo time();?>&iconMenu=" +iconNum; }
function gotoPageMenu()			{ window.location = "pageMenu.php"; }

var currentSelectionPrice = 0;
function onSelectionChanged(selNum)
{
	resetTimerGotoPageMenu();
	var selInfo = rhea.selection_getBySelNumber(selNum);
	
	currentSelectionPrice = 0;
	var price = "";
	if (rhea.isFreevend == 1)
		price = "FREEVEND";
	else if (rhea.isTestvend == 1)
		price = "TESTVEND";
	else
	{
		if (parseFloat(selInfo.price) <= 0)
			price = "";
		else
		{
			currentSelectionPrice = parseFloat(selInfo.price);
			price = selInfo.price +" " +rheaLang.LAB_CURRENCY_SIMBOL;
		}
	}
	if (price != "")
		price = " - " +price;

	var btnStart = rheaGetElemByID("btnStart");
	var btnStartNotAvail = rheaGetElemByID("btnStartNotAvail");
	if (selInfo.enabled == "0")
	{
		rheaHideElem(btnStart);
		rheaSetElemHTML(btnStartNotAvail, rheaLang.BTN_START_SELECTION_NOT_AVAIL);
		rheaShowElem(btnStartNotAvail);
	}
	else
	{
		rheaHideElem(btnStartNotAvail);
		rheaSetElemHTML(btnStart, rheaLang.BTN_START_SELECTION +price);
		rheaShowElem(btnStart);
	}
}

function rheaREST(api, callback)
{
	var url = "http://192.168.10.1/rhea/REST/" +api;
    var xmlhttp;
    xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function()
    {
        if (xmlhttp.readyState == 4)
        {
			if (xmlhttp.status == 200)
			{
				callback(xmlhttp.responseText);
			}
			else
			{
				//console.log ("ERR");
				//console.log (xmlhttp);
			}
		}
    }
    xmlhttp.open("GET", url, true);
    xmlhttp.send();
}

function REST_checkSelectionStatus()
{
	rheaREST("getSelStatus.php", function(data)
	{
		onSelectionStatusChanged(data, 1);
		setTimeout (function() { REST_checkSelectionStatus(); }, 1000);
	});
}

function onBtnStartSelection()
{
	resetTimerGotoPageMenu() ;
	rheaHideElem(rheaGetElemByID("beverageOptions"));
	rheaHideElem(rheaGetElemByID("divBtnHome")); //btn back
	rheaHideElem(rheaGetElemByID("divBtnSelectionInfo")); //btn selection info
	rheaShowElem(rheaGetElemByID("pleaseWait"));

	selThatWasStarted = detectCurrentSelection();


	console.log ("Starting sel num:" +selThatWasStarted);
	rhea.selection_start(selThatWasStarted);
	
	/*
	if (currentSelectionPrice <= 0)
	{
		//per bevande gratuite, uso il solito sistema
		console.log ("Starting sel num:" +selThatWasStarted);
		rhea.selection_start(selThatWasStarted);
	}
	else
	{
		//per quelle a pagamento, devo verificare il credito e poi avviare una transazione
		var sessionID = <?php 
		//echo $ses["sessionID"]
		?>;
		var app_userID = <?php 
		//echo $ses["app_userID"]
		?>;
		var price = GENCODER_encode(currentSelectionPrice);
		var selName = GENCODER_encode(MMI_getDisplayName(iconNum));
		rheaREST("tryPayAndStartSel.php?sesID=" +sessionID +"&app_userID=" +app_userID +"&selNum=" +selThatWasStarted +"&price=" +price +"&selName="+selName, function(data)  
		{ 
			if (data != "OK")
			{
				onSelectionKO();
				alert(data);
			}
			else
			{
				//comincia a pollare per conoscere lo stato di avanzamento della selezione
				REST_checkSelectionStatus();
			}
		});
	}
	*/
}

function resetTimerGotoPageMenu() 
{
	console.log ("resetTimerGotoPageMenu");
	if (null != timerGotoPageMenu)
		clearInterval(timerGotoPageMenu);
	timerGotoPageMenu = setInterval(gotoPageMenu, 15000);
}

</script>
</html>
