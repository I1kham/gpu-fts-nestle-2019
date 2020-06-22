//**************************************************************
function getFilenameFromPath(str)
{
	return str.split('\\').pop().split('/').pop();
}

//**************************************************************
function getUrlVars() 
{
	var vars = {}; 
	window.location.search.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) { vars[key] = value; }); 
	return vars;
}

//**************************************************************
function pleaseWait (bShowWait)
{
	if (bShowWait)
	{
		$("#main-content-wait-msg").html("");
		$("#main-content-wait").show();
		//$("#page-content").hide();
	}
	else
	{
		$("#main-content-wait").hide();
		$("#page-content").show();
	}
}

function pleaseWait_addMessage (msg)
{
	var h = $("#main-content-wait-msg").html();
	$("#main-content-wait-msg").html(h +"<br>" +msg);
}

function dataOra_getYYYYMMDD(sep)
{
	var d = new Date();
	var mm = (1 + d.getMonth()).toString();
	if (mm.length<2) mm = "0"+mm;
	
	var gg = d.getDate().toString();
	if (gg.length<2) gg = "0"+gg;
	
	return d.getFullYear() +sep +mm +sep +gg;
}

function dataOra_getHHMMSS(sep)
{
	var d = new Date();
	var hh = d.getHours().toString();
	if (hh.length<2) hh = "0"+hh;
	
	var mm = d.getMinutes().toString();
	if (mm.length<2) mm = "0"+mm;

	var ss = d.getSeconds().toString();
	if (ss.length<2) ss = "0"+ss;

	return hh +sep +mm +sep +ss;
}