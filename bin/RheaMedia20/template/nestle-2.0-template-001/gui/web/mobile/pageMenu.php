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
</head>

<body onLoad="rheaBootstrap()">
<div id="mainWrapper" class="wrapper">
	<div id="divCredit" class="header-credit">CREDIT <?php echo number_format ($ses["creditCurrent"], 2, ",", ".")?></div>
	<div class="header-sticky">
		<div id="divCPUMessage" style="display:none;"></div>	
	</div>

	<div id="divIconList"></div>

	<div class="clear">&nbsp;</div>
</div>




<!-- le opzioni di layout per questa pagina sono qui -->
<script src="config/allowedLang.js?dt=<?php echo time()?>"></script>
<script src="config/lang.js?dt=<?php echo time()?>"></script>
<script src="config/mainMenuIcons.js?dt=<?php echo time()?>"></script>
<script src="config/pageMenu.js?dt=<?php echo time()?>"></script>
<script src="config/MMI.js?dt=<?php echo time()?>"></script>
<script src="config/pageStandby.js?dt=<?php echo time()?>"></script>
<script src="js/rheaBootstrap.js?dt=<?php echo time()?>"></script>
<script language="javascript">

var timerGotoPageStandby = null;
function onRheaBootstrapFinished() 	{ lang_loadLang(lang_getCurLangISOCode()).then (onAfterLangLoaded); }
function onAfterLangLoaded() 
{
	rhea.onEvent_selectionAvailabilityUpdated = function () 
	{
		onSelectionAvailabilitUpdated();
	}

	rhea.onEvent_cpuStatus = function (statusID, statusStr, flag16)
	{
	}
	
	rhea.onEvent_cpuMessage = function(msg, importanceLevel)
	{
		var dCPU = rheaGetElemByID ("divCPUMessage");
		rheaSetElemHTML(dCPU, msg);
		rheaShowElem(dCPU);
	}

	setupIcons();
	rhea.requestGPUEvent(RHEA_EVENT_CPU_MESSAGE);
	rhea.requestGPUEvent(RHEA_EVENT_CPU_STATUS);
	resetTimerGotoPageStandby();
}

function onSelectionAvailabilitUpdated()
{
	var nIcon = MMI_getCount();
	for (var i=0; i<nIcon; i++)
	{
		var d = rheaGetElemByID("divIcon" +i);
		rheaRemoveClassToElem(d, "disabled");
		
		if (MMI_isEnabled(i) == 0)
			rheaAddClassToElem(d, "disabled");
	}
}						

function getTimeMSec() { return Date.now(); }

function resetTimerGotoPageStandby() 
{
	console.log ("resetTimerGotoPageStandby");
	if (null != timerGotoPageStandby)
		clearInterval(timerGotoPageStandby);
	timerGotoPageStandby = setInterval(gotoPageStandby, 20000);
}

function gotoPageStandby()  
{ 
	window.location = "pageDisconnect.html"; 
}


function setupIcons()
{
	var nIcon = MMI_getCount();
	var html = "";
	for (var i=0; i<nIcon; i++)
	{
		var iconDisplayName = MMI_getDisplayName(i);
		var image = "<img class='pageMenu_iconContainer_img' iconNum='" + i + "' draggable='false' src='" + MMI_getImgForPageMenu(i) + "' onclick='onIconMenuClicked(" +i +")'>";

		var cssForIconDisabled="";
		if (MMI_isEnabled(i) == 0)
			cssForIconDisabled=" disabled";
		html += "<div class='pageMenu_iconContainer w33 " +cssForIconDisabled +"'>"
			  + 	"<div style='position:absolute; top:0; left:0; width:100%; text-align:center'>"
			  + 		image
			  + 		"<div class='pageMenu_iconContainer_text' iconNum='" + i + "'>" + iconDisplayName + "</div>"
			  +		"</div>"
			  + 	"<div class='clear'></div>"
			  + "</div>";
	}

	var d = rheaGetElemByID("divIconList");
	rheaSetElemHTML(d, html);
}


function onIconMenuClicked(iconNum)
{
	resetTimerGotoPageStandby();
	if (MMI_isEnabled(iconNum))
		window.location = "pageConfirm.php?dt=<?php echo time();?>&iconMenu=" + iconNum;
}

function setLanguage(twoCharISOCode) 
{
	resetTimerGotoPageStandby();

	lang_loadLang(twoCharISOCode)
		.then(function (result) {
			window.location = window.location;
		})
		.catch(function (result) {
			rheaLog("AJAX::error => " + result);
		});
}



//--------------------------------------------------------
function divLangMenu_setup(currentLangISO)
{
	var html = "<ul class='langList'>";
	var eLang = allLang.split(",");
	for (var i=0; i<eLang.length; i++)
	{
		var iso = eLang[i];
		var text= allLangLocale[i];
		
		var cssClass = "btnSingleLang";
		if (iso == currentLangISO)
			cssClass += " selected";
		html += "<li><div id='divLangMenu_" +iso +"' class='" +cssClass +"' onclick='divLangMenu_select(\"" +iso +"\")'><img draggable='false' src='img/flags/" +iso +".png'><p>" +text +"</p></div></li>";
	}
	html += "</ul>";
	
	var d = rheaGetElemByID("divLangMenu_content");
	rheaSetElemHTML(d, html);
}
		
function divLangMenu_open()
{
	var d = rheaGetElemByID("divLangMenu");
	rheaShowElem(d);
}

function divLangMenu_close()
{
	var d = rheaGetElemByID("divLangMenu");
	rheaHideElem(d);
}

function divLangMenu_select(isoOfSelectedLang)
{
	var eLang = allLang.split(",");
	for (var i=0; i<eLang.length; i++)
	{
		var iso = eLang[i];
		var d = rheaGetElemByID("divLangMenu_" +iso);
		rheaRemoveClassToElem(d, "selected");
		if (iso == isoOfSelectedLang)
		{
			rheaAddClassToElem(d, "selected");
			setLanguage(iso);
		}
	}
	divLangMenu_close();
}
</script>


</html>
