<?php if (!session_id()) session_start();
require_once "../REST/DBConn.php";
require_once "../REST/common.php";
$db = new DBConn();
if (!isset($_SESSION["sesID"]))
{
	$ses = sessionGetCurrent ($db);
	$_SESSION["sessionID"] = $ses["sessionID"];
	$_SESSION["app_userID"] = $ses["app_userID"];
	$_SESSION["app_numDecimaliPrezzo"] = $ses["app_numDecimaliPrezzo"];
	$_SESSION["app_simboloValuta"] = $ses["app_simboloValuta"];
}

$sessionID = $_SESSION["sessionID"];
$app_userID = $_SESSION["app_userID"];
sessionUpdateTimestamp ($db, $sessionID);
$simboloValuta = $_SESSION["app_simboloValuta"];
if ($simboloValuta != "")
	$simboloValuta = "&nbsp;" .$simboloValuta;
$_SESSION["currentCredit"] = number_format (sessionGetCurrentCredit($db, $sessionID), $_SESSION["app_numDecimaliPrezzo"], ",", ".") .$simboloValuta;
?>
<html>
<head>
	<title>GUI-TP</title>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
	<link href="style.css?dt=<?php echo time();?>" rel="stylesheet" type="text/css">
</head>

<body onLoad="rheaBootstrap()">
<div id="mainWrapper" class="wrapper">
	<!--<div id="divCredit">CREDIT -->
		<?php 
		//echo $_SESSION["currentCredit"]; 
		?>
	<!--</div>-->
	<div id="divCPUMessage">&nbsp;</div>
	<div id="divIconList"></div>
	<div id="divPleaseWait" style="display:none"><br><br><br><center><img src="img/animationRound.gif"></center></div>

	<div class="clear">&nbsp;</div>
</div>

<div id="divAlert">
	<div id="divAlert_text"></div>
	<div id="divAlert_close" onclick="myAlert_close()">CLOSE</div>
</div>


<script src="config/allowedLang.js"></script>
<script src="config/mainMenuIcons.js"></script>
<script src="js/rhea.js?dt=<?php echo time();?>"></script>
<script src="js/gjs.js"></script>
<script language="javascript">

var ASK_CPU_STATUS_EVERY_Msec = 500;
var ASK_12LED_STATUS_EVERY_Msec = 600;
var timerGotoPageDisconnect = null;
var statusOfLEDBtn = [];
var iSelRunning = -1;

function rheaBootstrap()				{ lang_loadLang(lang_getCurLangISOCode()).then (onAfterLangLoaded); }
function onAfterLangLoaded() 
{
	setupIcons();
	resetTimerGotoPageDisconnect();
	ask12LEDStatus();
	askCPUStatus();
}

function resetTimerGotoPageDisconnect() 
{
	console.log ("resetTimerGotoPageDisconnect");
	if (null != timerGotoPageDisconnect)
		clearInterval(timerGotoPageDisconnect);
	timerGotoPageDisconnect = setInterval(gotoPageDisconnect, 30000);
}

function gotoPageDisconnect()  
{ 
	window.location = "pageDisconnect.html?dt=" +Rhea_getTimeStamp();
}


function setupIcons()
{
	var nIcon = MMI_getCount();
	var html = "";
	
	for (var row=0; row<3; row++)
	{
		for (var col=0; col<4; col++)
		{
			var btnNum = (row) +3*col;
			
			//il nome selezione è indicato nel file lang/GB.js
			//Se il nome è "", si prende il nome di CPU
			var iconDisplayName = rheaSelName[btnNum];
			if (iconDisplayName == "")
				iconDisplayName = Rhea_getCachedSelectionName(btnNum+1);
			else
				Rhea_setCachedSelectionName(btnNum+1, iconDisplayName);
				
			
			var iconPrice = Rhea_getCachedSelectionPrice(btnNum+1);
//console.log ("cached sel name " +(btnNum+1) +" [" +iconDisplayName +"]");
			var image = "<img class='pageMenu_iconContainer_img' iconNum='" + btnNum + "' draggable='false' src='" + MMI_getImgForPageMenu(btnNum) + "' onclick='onIconMenuClicked(" +btnNum +")'>";

			var cssForIconDisabled="";
			if (rheaLEDBtn_isOn(btnNum) == 0)
			{
				cssForIconDisabled=" disabled";
				statusOfLEDBtn[btnNum] = 0;
			}
			else
				statusOfLEDBtn[btnNum] = 1;
				
			var divName ="btnSel" +(btnNum+1);
			var divPrice ="btnPrice" +(btnNum+1);
			html += "<div id='" +divName +"' class='pageMenu_iconContainer w33 " +cssForIconDisabled +"'>"
				  + 	"<div style='position:absolute; top:0; left:0; width:100%; text-align:center'>"
				  + 		image
				  + 		"<div id='" +divName +"_label' class='pageMenu_iconContainer_text' iconNum='" + btnNum + "'>" + iconDisplayName + "</div>"
				  + 		"<div id='" +divPrice +"_label' class='pageMenu_iconContainer_text' style='padding-top:4px; color:#aaa;' iconNum='" + btnNum + "'>" + iconPrice + "</div>"
				  +		"</div>"
				  + 	"<div class='clear'></div>"
				  + "</div>";
		}
	}
	html += "<div class='clear'></div>";
	
	var d = rheaGetElemByID("divIconList");
	rheaSetElemHTML(d, html);
	ask12SelPrices();
	ask12Selnames();
}

function on12LEDUpdated()
{
	for (var i=0; i<12; i++)
	{
		var divName ="btnSel" +(i+1);
		var d = rheaGetElemByID(divName);
		if (rheaLEDBtn_isOn(i))
		{
			if (!statusOfLEDBtn[i])
			{
				rheaRemoveClassToElem(d, "disabled");
				statusOfLEDBtn[i] = 1;
			}
		}
		else
		{
			if (statusOfLEDBtn[i])
			{
				rheaAddClassToElem(d, "disabled");
				statusOfLEDBtn[i] = 0;
			}
		}
	}
}


function askCPUStatus()
{
	rheaREST("getCPULCDMsg.php", function(data) 
	{
		rheaSetElemHTML (rheaGetElemByID("divCPUMessage"), data);
		setTimeout(askCPUStatus, ASK_CPU_STATUS_EVERY_Msec);
	});	
	
}

function ask12LEDStatus()
{
	rheaLEDBtn_askStatus(function() 
	{ 
		on12LEDUpdated(); 
		setTimeout(ask12LEDStatus, ASK_12LED_STATUS_EVERY_Msec);  
	});
}

function ask12Selnames()
{
	rheaREST("get12SelNames.php", function(data) 
	{
		var names = data.split("|");
		var n = names.length;
		if (n>12)
			n=12;
		for (var i=0; i<n; i++)
		{
			var selName = names[i].trim();
			if (selName != "")
			{
				if (rheaSelName[i] == "")
				{
					rheaSetElemHTML (rheaGetElemByID("btnSel" +(i+1) +"_label"), selName);
					Rhea_setCachedSelectionName (i+1, selName);
				}
			}
		}
	});	
}

function ask12SelPrices()
{
	rheaREST("get12SelPrices.php", function(data) 
	{
		var prices = data.split("|");
		var n = prices.length;
		if (n>12)
			n=12;
		for (var i=0; i<n; i++)
		{
			rheaSetElemHTML (rheaGetElemByID("btnPrice" +(i+1) +"_label"), prices[i]);
			Rhea_setCachedSelectionPrice (i+1, prices[i]);
		}
	});	
}

function onIconMenuClicked(iconNum)
{
	resetTimerGotoPageDisconnect();
	if (!rheaLEDBtn_isOn(iconNum))
		return;
		
	
	rheaHideElem (rheaGetElemByID("divIconList"));
	rheaShowElem (rheaGetElemByID("divPleaseWait"));

	iSelRunning = iconNum+1;
	
	var sessionID = <?php echo $sessionID?>;
	var app_userID = <?php echo $app_userID?>;
	var price = GENCODER_encode(Rhea_getCachedSelectionPrice(iSelRunning));
	var selName = GENCODER_encode(Rhea_getCachedSelectionName(iSelRunning));
	rheaREST("tryPayAndStartSel.php?sesID=" +sessionID +"&app_userID=" +app_userID +"&selNum=" +iSelRunning +"&price=" +price +"&selName="+selName, function(data)  
	{ 
		if (data == "OK")
			checkSelectionStatus(); 
		else
		{
			onSelFinishedKO();
			myAlert(data);
		}
	});
}

function checkSelectionStatus()
{
	rheaREST("getSelStatus.php", function(data)
	{
		switch (data)
		{
		default: //com error
			console.log ("com error, data[" +data +"]");
			onSelFinishedKO();
			return;
			
		case "1": //waiting payment
			console.log ("waiting payment");
			break;

		case "2": //delivering in progress
		case "5": //delivering in progress
			console.log ("delivering in progress");
			window.location = "pageSelInProgress.html?selNum=" +iSelRunning +"&dt=" +Rhea_getTimeStamp();; 
			return;

		case "3": //finished OK
			console.log ("finished OK");
			onSelFinishedKO();
			break;
			
		case "4": //finished KO
			console.log ("finished KO");
			onSelFinishedKO();
			return;
			
		}
		
		setTimeout (function() { checkSelectionStatus(); }, 1000);
	});
	
	
}

function onSelFinishedKO()
{
	resetTimerGotoPageDisconnect();
	iSelRunning = -1;
	rheaShowElem (rheaGetElemByID("divIconList"));
	rheaHideElem (rheaGetElemByID("divPleaseWait"));
}

function myAlert(msg)		{ rheaSetElemHTML(rheaGetElemByID("divAlert_text"), msg); rheaShowElem(rheaGetElemByID("divAlert")); }
function myAlert_close() 	{ rheaHideElem(rheaGetElemByID("divAlert")); }
</script>
</html>
