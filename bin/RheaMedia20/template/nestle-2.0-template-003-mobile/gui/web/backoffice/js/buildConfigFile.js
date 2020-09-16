/*********************************************************
 * Preview box utils
 */
function buildConfigFile_fixStringFromTextArea (s)			{ if(s=="NULL") return ""; s = s.replace(/"/g, '&#34;'); return s.replace(/\n/g, '<br>'); }
function buildConfigFile_fixStringFromTextbox (s)			{ if(s=="NULL") return ""; return s.replace(/"/g, '&#34;'); }

var previewBox_built = 0;
var previewBox_saveAs = null;
async function previewFrame_open()
{
	$("#mainWrapper").hide();
	var d = $("#previewBox");
	if (0 == previewBox_built)
	{
		previewBox_built = 1;
		var html = "<iframe id='previewFrame' class='previewBox_frame' src='' border='0'></iframe>"
					+"<div class='previewBox_close' onclick='previewFrame_close()'><img src='img/close32x32.png'></div>";
					
		var yyyymmddhhmmss = isATemporaryPath();
		if (yyyymmddhhmmss!="")
		{
			html += "<div class='previewBox_saveAs' onclick='previewFrame_saveAs(\"" +yyyymmddhhmmss +"\")'>EXPORT / SAVE AS</div>"
				   +"<div id='preview_fileSaveAs' class='fileBrowser'></div>";
		}
		d.html(html);
		
		previewBox_saveAs = new FileSaveAs("preview_fileSaveAs");
	}
	
	d.show();
	var f = document.getElementById("previewFrame");
	f.src = "../startup.html";
}

function previewFrame_close()
{
	$("#mainWrapper").show();
	var f = document.getElementById("previewFrame");
	f.src = "previewClose.html";
	$("#previewBox").hide();
}

function previewFrame_saveAs(tempFolderName)
{
	previewBox_saveAs.open("c:", previewFrame_saveAs_onFinished, "Please selected a <b>DESTINATION FOLDER</b> then press the <span class='fileBrowser_saveBtn'>SAVE HERE</span> button located to the right of the screen.<br>The GUI will be copied in the choosen folder.");
}

function previewFrame_saveAs_onFinished(dstPath)
{
	console.log ("[" +dstPath +"]");
	if (dstPath == "")
		return;
	
	previewFrame_close();
	pleaseWait(1);

	var srcTempFolderName = isATemporaryPath();
	rhea.ajax ("taskSpawn", { "name" : "exportMobileTPGUIToUserFolder", "params":srcTempFolderName+"§"+dstPath})
				.then( function(result)
				{
					var taskID = parseInt(result);
					console.log ("new task id[" +taskID +"]");
					setTimeout (function(){ previewFrame_saveAs_taskSave_query(taskID);}, 400);
						
				});
}	

function previewFrame_saveAs_taskSave_query (taskID)
{
	rhea.ajax ("taskStatus", { "id" : taskID})
				.then( function(result)
				{
					var obj = JSON.parse(result);
					console.log ("[" +taskID +"] status[" +obj.status +"], msg[" +obj.msg +"]");
					
					pleaseWait_addMessage (obj.msg);
					if (obj.status == "0")
						previewFrame_saveAs_saved(obj.msg);
					else
						setTimeout (function(){ previewFrame_saveAs_taskSave_query(taskID);}, 1000);
				});
}

function previewFrame_saveAs_saved(result)
{
	if (result=="OK")
		alert ("The GUI has been succesfully saved");
	else
		alert ("ERROR: " +result);	
	pleaseWait(0);
}

async function onBtnBuildAndPreview()
{
	if (null == db)
	{
		var curPath = rheaGetAbsolutePhysicalPath();
		db = new RheaDB (curPath +"/guidb.db3");
	}
	
	pleaseWait(1);
	await buildConfigFile_prepareEverything(db, historyID)
	pleaseWait(0);
	previewFrame_open();
}


/*********************************************************
 *  raccoglie tutte le info dal DB e prepara tutti i js
 *	necessari alla GUI.
 *	Lo fa solo se la data di lastBuild è < della dataUM
 */
async function buildConfigFile_prepareEverything(db, historyID)
{
console.time("prepareEverything");
	let rst = await db.q("SELECT DataUM,DataLastBuild FROM history WHERE ID=" +historyID);
	if (rst.hasError() != "")
	{
		alert ("buildConfigFile_prepareEverything::ERR => " +rst.hasError());
		return;
	}
	if (rst.valByColName(0, "DataLastBuild") >= rst.valByColName(0, "DataUM"))
		return;
	
console.time("language files");	
	pleaseWait_addMessage("creating language files...");
	await buildConfigFile_allowedLang(db, historyID);
console.timeEnd("language files");	

console.time("main menu icons");	
	pleaseWait_addMessage("creating main menu icons files...");
	await buildConfigFile_mainMenuIcons(db, historyID);
console.timeEnd("main menu icons");	
		
console.time("translation");	
	pleaseWait_addMessage("creating translation files...");
	await buildConfigFile_translation(db, historyID);
console.timeEnd("translation");	
	
	await db.exec ("UPDATE history SET DataLastBuild='" +buildConfigFile_getAAAAMMGGHHMMSS() +"' WHERE ID=" +historyID);
console.timeEnd("prepareEverything");
}

//*********************************************************
function buildConfigFile_getAAAAMMGGHHMMSS()
{
	var currentdate = new Date(); 
	var dd = currentdate.getDate(); if (dd<10) dd = "0"+dd;
	var mm = (currentdate.getMonth()+1); if (mm<10) mm = "0"+mm;
	var yyyy = currentdate.getFullYear();
	var hh = currentdate.getHours(); if (hh<10) hh = "0"+hh;
	var minutes = currentdate.getMinutes(); if (minutes<10) minutes = "0"+minutes;
	var ss = currentdate.getSeconds(); if (ss<10) ss = "0"+ss;
	return yyyy.toString() +mm.toString() +dd.toString() +hh.toString() +minutes.toString() +ss.toString();
}


//*********************************************************
/*var buildConfigFile_saveState = 0;
function buildConfigFile_onSaveFinished()
{
	buildConfigFile_saveState = 1;
}
*/

function buildConfigFile_saveAs(what, path, filename)
{
	var buildConfigFile_saveState = 0;
	var BCFSAVEAS_TIME_MSEC = 5;
	return new Promise( function(resolve, reject) 
						{
							var enc = new TextEncoder(); 
							var uint8array = enc.encode(what);
							buildConfigFile_saveState = 0;
							//saveAs (uint8array.buffer, path, filename, buildConfigFile_onSaveFinished);
							saveAs (uint8array.buffer, path, filename, function() {buildConfigFile_saveState=1;});
							
							//attendo la fine del salvataggio
							var timeoutMs = 20000;
							var waitSave = function()	{
															if (buildConfigFile_saveState == 1)
																resolve(1);
															else if ((timeoutMs -= BCFSAVEAS_TIME_MSEC) < 0)
															{
																reject ("timed out 'waitSave'");
																reject(0);
															}
															else
																setTimeout(waitSave, BCFSAVEAS_TIME_MSEC);
														}

							setTimeout(waitSave, BCFSAVEAS_TIME_MSEC)								
						});
}


/*********************************************************
 * crea il file config/allowedLang.js
 */
async function buildConfigFile_allowedLang(db, his_id)
{
	var result = "var allLang=\"GB\"; var defaultLang=\"GB\";allLangLocale=[\"English\"];";		
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "allowedLang.js");
}


/*********************************************************
 * crea il file config/mainMenuIcons.js
 */
async function buildConfigFile_mainMenuIcons(db, his_id)
{
console.time("  main menu icons [rst]");
	let rst = await db.q("SELECT selNum,pageMenuImg FROM pagemenu_mmi WHERE HIS_ID=" +his_id +" ORDER BY selNum");
console.timeEnd("  main menu icons [rst]");	

	var result = "var rheaMainMenuIcons = [";
	for (var row=0; row<rst.getNumRows(); row++)
	{
		result += "{selNum:" +rst.valByColName(row, "selNum")
					+",pageMenuImg:\"" +rst.valByColName(row, "pageMenuImg") +"\""
					+",pageConfirmImg:\"" +rst.valByColName(row, "pageMenuImg") +"\""
				+"},\n";
	}
	result +="];";

console.time("  main menu icons [saveas]");	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "mainMenuIcons.js");
console.timeEnd("  main menu icons [saveas]");		
}


/*********************************************************
 * crea il file config/lang/GB.js e GB_selDescr.js
 */
async function buildConfigFile_translation (db, his_id)
{
	var result = "";
	var resultDescr = "";
	
	let rst = await db.q("SELECT UID,Message FROM lang WHERE What='MSG' AND ISO='GB'");
	result += "var rheaLang = {";
	for (var row=0; row<rst.getNumRows(); row++)
		result += rst.valByColName(row, "UID") +":\"" +buildConfigFile_fixStringFromTextArea(rst.valByColName(row, "Message")) +"\",";
	result +="ARRAY_END:\"\"};\n";
	
	rst = await db.q("SELECT name,descr FROM pagemenu_mmi WHERE HIS_ID=" +his_id +" ORDER BY selNum");
	result += "var rheaSelName=[";
	resultDescr = "var rheaSelDescr=[";
	for (var row=0; row<rst.getNumRows(); row++)
	{
		result += "\"" +buildConfigFile_fixStringFromTextbox(rst.valByColName(row, "name")) +"\",";
		resultDescr += "\"" +buildConfigFile_fixStringFromTextArea(rst.valByColName(row, "descr")) +"\",";
	}
	result += "];";
	resultDescr += "];";
	

console.time("  translation [saveas] GB.js");	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config/lang", "GB.js");
console.timeEnd("  translation [saveas] GB.js");		


console.time("  translation [saveas] GB_selDescr.js");	
	await buildConfigFile_saveAs (resultDescr, rheaGetAbsolutePhysicalPath()+"/../config/lang", "GB_selDescr.js");
console.timeEnd("  translation [saveas] GB_selDescr.js");		
}


