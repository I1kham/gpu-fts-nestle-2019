/*********************************************************
 * Preview box utils
 */
function setCharAt(str,index,chr) {
	if(index > str.length-1) return str;
	return str.substr(0,index) + chr + str.substr(index+1);
}
	
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
	f.src = "../startup.html?rheamedia2preview=1";
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
	rhea.ajax ("taskSpawn", { "name" : "exportGUIToUserFolder", "params":srcTempFolderName+"§"+dstPath})
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

console.time("page standby");		
	pleaseWait_addMessage("creating page standby files...");
	await buildConfigFile_pageStandby(db, historyID);
console.timeEnd("page standby");	
	

console.time("page menu");		
	pleaseWait_addMessage("creating page menu files...");
	await buildConfigFile_pageMenu(db, historyID);
console.timeEnd("page menu");	

console.time("page confirm finished");
	pleaseWait_addMessage("creating page confirm files..");
	await buildConfigFile_pageConfirm(db, historyID);
console.timeEnd("page confirm finished");	


console.time("page sel in progress");		
	pleaseWait_addMessage("creating page sel in progress files...");
	await buildConfigFile_pageSelInProgress(db, historyID);
console.timeEnd("page sel in progress");	

console.time("page sel finished");		
	pleaseWait_addMessage("creating page sel finished files..");
	await buildConfigFile_pageSelFinished(db, historyID);
console.timeEnd("page sel finished");	


console.time("main menu icons");	
	pleaseWait_addMessage("creating main menu icons files...");
	await buildConfigFile_mainMenuIcons(db, historyID);
console.timeEnd("main menu icons");	
		
console.time("translation");	
	pleaseWait_addMessage("creating translation files...");
	await buildConfigFile_translation(db, historyID);
console.timeEnd("translation");	
	
console.time("info for menu prog");	
	pleaseWait_addMessage("exporting info for menu prog...");
	await buildConfigFile_menuProg(db, historyID);
console.timeEnd("info for menu prog");	
	
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
 * crea il file config/pageConfirm.js
 */
async function buildConfigFile_pageConfirm (db, his_id)
{
	let rst = await db.q("SELECT ValueA FROM other WHERE UID='pageConfirm' AND ISO='xx' AND What='timeToMain'");
	if (rst.getNumRows() == 0)
	{
		await db.exec ("INSERT INTO other (UID,ISO,What,ValueA) VALUES('pageConfirm','xx','timeToMain', '15000')");
		rst = await db.q("SELECT ValueA FROM other WHERE UID='pageConfirm' AND ISO='xx' AND What='timeToMain'");
	}
	var timeToMainMSec = rst.valByColName(0, "ValueA");
	
	rst = await db.q("SELECT ValueA FROM other WHERE UID='pageConfirm' AND ISO='xx' AND What='alipayAbortSec'");
	if (rst.getNumRows() == 0)
	{
		await db.exec ("INSERT INTO other (UID,ISO,What,ValueA) VALUES('pageConfirm','xx','alipayAbortSec', '60')");
		rst = await db.q("SELECT ValueA FROM other WHERE UID='pageConfirm' AND ISO='xx' AND What='alipayAbortSec'");
	}
	var alipayAbortSec = rst.valByColName(0, "ValueA");	
	
	var result = "var pageConfirmOptions={timeToMainMSec:" +timeToMainMSec +",abortSec:" +alipayAbortSec +"};";	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "pageConfirm.js");
}


/*********************************************************
 * crea il file config/pageSelFinished.js
 */
async function buildConfigFile_pageSelFinished (db, his_id)
{
	let rst = await db.q("SELECT ValueA FROM other WHERE UID='pageSelFinished' AND ISO='xx' AND What='timeToMain'");
	if (rst.getNumRows() == 0)
	{
		await db.exec ("INSERT INTO other (UID,ISO,What,ValueA) VALUES('pageSelFinished','xx','timeToMain', '2000')");
		rst = await db.q("SELECT ValueA FROM other WHERE UID='pageSelFinished' AND ISO='xx' AND What='timeToMain'");
	}
	var result = "var selFinishedPageOptions={timeToGoBack:" +rst.valByColName(0, "ValueA") +"};";	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "pageSelFinished.js");
}


/*********************************************************
 * crea il file config/pageStandby.js
 */
async function buildConfigFile_pageStandby (db, his_id)
{
console.time("  page standby[rst1]");
	let rst = await db.q("SELECT * FROM pageStandby WHERE HIS_ID=" +his_id);
console.timeEnd("  page standby[rst1]");	
	var bg_a = (parseFloat(rst.valByColName(0, "overlay_bgcolor_A")) / 100.0).toFixed(2);
	var result = "var standbyPageOptions={"
				+"enabled:" +rst.valByColName(0, "pageEnabled")
				+",overlay_visible:" +rst.valByColName(0, "overlay_visible")
				+",overlay_bgcolor:\"rgba(" +rst.valByColName(0, "overlay_bgcolor_R") +"," +rst.valByColName(0, "overlay_bgcolor_G") +"," +rst.valByColName(0, "overlay_bgcolor_B") +"," +bg_a +")\""
				+",overlay_textColor:\"rgb(" +rst.valByColName(0, "overlay_textColor_R") +"," +rst.valByColName(0, "overlay_textColor_G") +"," +rst.valByColName(0, "overlay_textColor_B") +")\""
				+",gallery_timeToNextImageMSec:" +rst.valByColName(0, "gallery_timeToNextImageMSec")
				+",gallery_images:["	
	
	
console.time("  page standby[rst2]");	
	rst = await db.q("SELECT filename FROM pageStandby_images WHERE HIS_ID=" +his_id +" ORDER BY PROGR");
console.timeEnd("  page standby[rst2]");
	for (var r=0; r<rst.getNumRows(); r++)
	{
		if (r>0)
			result += ",";
		result += "\"" +rst.valByColName(r,"filename") +"\"";
	}
	result += "]};";
	
console.time("  page standby[saveas]");	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "pageStandby.js");
console.timeEnd("  page standby[saveas]");		
}

/*********************************************************
 * crea il file config/pageSelInProgress.js
 */
async function buildConfigFile_pageSelInProgress (db, his_id)
{
	let rst = await db.q("SELECT * FROM pageSelInProgress WHERE HIS_ID=" +his_id);
	var result = "var rheaPageOptions={"
				+"gallery_timeToNextImageMSec:" +rst.valByColName(0, "gallery_timeToNextImageMSec")
				+",gallery_images:["	
		
	rst = await db.q("SELECT filename FROM pageSelInProgress_images WHERE HIS_ID=" +his_id +" ORDER BY PROGR");
	for (var r=0; r<rst.getNumRows(); r++)
	{
		if (r>0)
			result += ",";
		result += "\"" +rst.valByColName(r,"filename") +"\"";
	}
	result += "]};";
	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "pageSelInProgress.js");
}


/*********************************************************
 * crea il file config/pageMenu.js
 */
async function buildConfigFile_pageMenu(db, his_id)
{
	let rst = await db.q("SELECT * FROM pagemenu WHERE HIS_ID=" +his_id);
	var result = "var mainMenuPageOptions={"
				+"icon_numRow:" +rst.valByColName(0, "icon_numRow")
				+",icon_numIconPerRow:" +rst.valByColName(0, "icon_numIconPerRow")
				+",gotoPageStandbyAfterMSec:" +rst.valByColName(0, "gotoPageStandbyAfterMSec")
				+",oneClickSelection:" +rst.valByColName(0, "oneClickSelection")
				+"};";
	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "pageMenu.js");
}

/*********************************************************
 * crea il file config/allowedLang.js
 */
async function buildConfigFile_allowedLang(db, his_id)
{
	let rst = await db.q("SELECT ISO,LocalName FROM allLanguages ORDER BY IndexVis")
	var allLang = [];
	for (var r=0; r<rst.getNumRows(); r++)
	{
		var iso = rst.valByColName(r, "ISO");
		var s = rst.valByColName(r, "LocalName");
		
		var o = {"iso": iso, "lngDescription":s};
		allLang.push (o);
	}
	
	
	rst = await db.q("SELECT * FROM generalset WHERE HIS_ID=" +his_id);
	var result = "var allLang=\"" +rst.valByColName(0, "allowedLang") +"\"; var defaultLang=\"" +rst.valByColName(0, "defaultLang") +"\";";
			
	var locale="";
	var e = rst.valByColName(0, "allowedLang").split(",");
	for (var i=0; i<e.length; i++)
	{
		var iso = e[i];
		for (var i1=0; i1<allLang.length; i1++)
		{
			if (iso == allLang[i1].iso)
			{
				if (locale!="")
					locale+=",";
				locale += "\"" +allLang[i1].lngDescription +"\"";
				break;
			}
		}
	}
			
			
	result += "allLangLocale=[" +locale +"];";		
	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "allowedLang.js");
}


/*********************************************************
 * crea il file config/mainMenuIcons.js
 */
async function buildConfigFile_mainMenuIcons(db, his_id)
{
console.time("  main menu icons [rst]");
	let rst = await db.q("SELECT * FROM pagemenu_mmi WHERE HIS_ID=" +his_id +" ORDER BY PROGR");
console.timeEnd("  main menu icons [rst]");	

	var result = "var rheaMainMenuIcons = [";
	for (var r=0; r<rst.getNumRows(); r++)
		result += "{" +priv_buildConfigFile_mainMenuIcons_2 (rst, r) +"},\n";
	result +="];";

console.time("  main menu icons [saveas]");	
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config", "mainMenuIcons.js");
console.timeEnd("  main menu icons [saveas]");		
}

//**************************************************************
function priv_buildConfigFile_mainMenuIcons_cupSizeLetterToNumber (cupSize)
{
	switch (cupSize) { case 'M': return 1; case 'L': return 2; default: return 0; }
}

/**************************************************************
//      0  1  2  3           4  5  6  7            8  9  10 11
//small NN NY YN YY   medium NN NY YN YY     large NN NY YN YY
*/
function priv_buildConfigFile_mainMenuIcons_2 (rst, row)
{
	if (rst.valByColName(row, "selNum") > 0)
	{
		//selezione senza cup customization
		var result = "selNum:" +rst.valByColName(row, "selNum")
					+",size:\"" +rst.valByColName(row, "allowedCupSize") +"\""
					+",pageMenuImg:\"" +rst.valByColName(row, "pageMenuImg") +"\""
					+",hidden:\"" + (rst.hasColName("HIDDEN_SELECTION")? parseInt(rst.valByColName(row, "HIDDEN_SELECTION")) : 0) +"\""
					+",pageConfirmImg:\"" +rst.valByColName(row, "pageConfirmImg") +"\"";
		return result;
	}
	else
	{
		//selezione con cup customization
		var selections = rst.valByColName(row, "linkedSelection").split(",");
		var linkedSelection = "["
								+"[[" +selections[0] +"," +selections[1] +"],"		//grinder0 cup0 shot0, grinder0 cup0 shot1
								+ "[" +selections[4] +"," +selections[5] +"],"		//grinder0 cup1 shot0, grinder0 cup1 shot1
								+ "[" +selections[8] +"," +selections[9] +"]],"		//grinder0 cup2 shot0, grinder0 cup2 shot1
								
								+"[[" +selections[2] +"," +selections[3] +"],"		//grinder1 cup0 shot0, grinder1 cup0 shot1
								+ "[" +selections[6] +"," +selections[7] +"],"		//grinder1 cup1 shot0, grinder1 cup1 shot1
								+ "[" +selections[10] +"," +selections[11] +"]],"	//grinder1 cup2 shot0, grinder1 cup2 shot1
							+"]";
		
		
		var defSel="[";
		var defaultSelection = ""; ////grinder, cup-size, shot-type
		var s = rst.valByColName(row, "defaultSelectionOption").substr(0,1); if (s == "x") defaultSelection+="N"; else defaultSelection+=s;
			s = rst.valByColName(row, "defaultSelectionOption").substr(1,1); if (s == "x") defaultSelection+="N"; else defaultSelection+=s;
			defaultSelection += rst.valByColName(row, "defaultSelectionOption").substr(2,1);
		if (defaultSelection.substr(0,1) == "N")
		{
			if (defaultSelection.substr(1,1) == "N")
			{
				//NNx
				defSel +="0," +priv_buildConfigFile_mainMenuIcons_cupSizeLetterToNumber (defaultSelection.substr(2,1)) +",0";
			}
			else
			{
				//NYx
				defSel +="0," +priv_buildConfigFile_mainMenuIcons_cupSizeLetterToNumber (defaultSelection.substr(2,1)) +",1";
			}
		}
		else
		{
			if (defaultSelection.substr(1,1) == "N")
			{
				//YNx
				defSel +="1," +priv_buildConfigFile_mainMenuIcons_cupSizeLetterToNumber (defaultSelection.substr(2,1)) +",0";
			}
			else
			{
				//YYx
				defSel +="1," +priv_buildConfigFile_mainMenuIcons_cupSizeLetterToNumber (defaultSelection.substr(2,1)) +",1";
			}		
		}
		defSel+="]";
		
		
		var result = "selNum:" +rst.valByColName(row, "selNum")
					+",size:\"" +rst.valByColName(row, "allowedCupSize") +"\""
					+",pageMenuImg:\"" +rst.valByColName(row, "pageMenuImg") +"\""
					+",pageConfirmImg:\"" +rst.valByColName(row, "pageConfirmImg") +"\""
					+",dblShot:" +rst.valByColName(row, "optionBEnabled")
					+",grinder2:" +rst.valByColName(row, "optionAEnabled")
					+",linkedSelection:" +linkedSelection
					+",hidden:" + (rst.hasColName("HIDDEN_SELECTION")? parseInt(rst.valByColName(row, "HIDDEN_SELECTION")) : 0)
					+",defaultSelection:" +defSel;
		
		return result;
	}	

}


/*********************************************************
 * crea il file config/lang/xx.js per ogni lingua supportata
 */
async function buildConfigFile_translation (db, his_id)
{
	let rst = await db.q("SELECT * FROM generalset WHERE HIS_ID=" +his_id);
	var allLangString = rst.valByColName(0, "allowedLang");
	var allLang = allLangString.split(",");
	var defaultLang = rst.valByColName(0, "defaultLang");
	
console.time("translation 01");
	rstSelection = await db.q("SELECT UID,selNum,bCustomBtnCupAppeance FROM pagemenu_mmi WHERE HIS_ID=" +his_id +" ORDER BY PROGR");
	let rstStdbyMessage = await db.q("SELECT ISO,Message FROM lang WHERE UID='STDBY' AND What='MSG'");
	let rstBTN_STOP = await db.q("SELECT ISO,Message FROM lang WHERE UID='BTN_STOP' AND What='MSG'");
	let rstBTN_ABORT = await db.q("SELECT ISO,Message FROM lang WHERE UID='BTN_ABORT' AND What='MSG'");
	let rstLAB_PLEASE_SCAN_QRCODE = await db.q("SELECT ISO,Message FROM lang WHERE UID='LAB_PLEASE_SCAN_QRCODE' AND What='MSG'");
	let rstBTN_START_SELECTION = await db.q("SELECT ISO,Message FROM lang WHERE UID='BTN_START_SELECTION' AND What='MSG'");
	let rstBTN_START_SELECTION_NOT_AVAIL = await db.q("SELECT ISO,Message FROM lang WHERE UID='BTN_START_SELECTION_NOT_AVAIL' AND What='MSG'");
	let rstBTN_START_OFFLINE = await db.q("SELECT ISO,Message FROM lang WHERE UID='BTN_START_OFFLINE' AND What='MSG'");
	let rstLAB_CURRENCY_SIMBOL = await db.q("SELECT ISO,Message FROM lang WHERE UID='LAB_CURRENCY_SIMBOL' AND What='MSG'");
	let rstLAB_YOUR_DRINK_IS_READY = await db.q("SELECT ISO,Message FROM lang WHERE UID='LAB_YOUR_DRINK_IS_READY' AND What='MSG'");
	let rstLAB_YOUR_DRINK_IS_BEING_PREPARED = await db.q("SELECT ISO,Message FROM lang WHERE UID='LAB_YOUR_DRINK_IS_BEING_PREPARED' AND What='MSG'");
	let rstFOOTER_BTNS = await db.q("SELECT What,Message FROM lang WHERE What='langButtonImg' OR What='promoButtonImg'");
	let rstLearnMore = await db.q("SELECT ISO,bgImage FROM pageLearnMore WHERE HIS_ID=" +his_id);		
console.timeEnd ("translation 01");

	//cup custom btn appearance
console.time("translation 02");	
	var defCupAppearanceA = "A";
	var defCupAppearanceB = "C";
	let rstCupAppearance = await db.q("SELECT AppearanceA,AppearanceB FROM cupcustombtn WHERE MMI='-1' AND HIS_ID=" +his_id);
	if (rstCupAppearance.getNumRows()> 0)
	{
		defCupAppearanceA = rstCupAppearance.valByColName(0, "AppearanceA");
		defCupAppearanceB = rstCupAppearance.valByColName(0, "AppearanceB");
	}
	rstCupAppearance = null;
	var cupCustomBtnAppearance = [];
	for (var iSel=0; iSel<rstSelection.getNumRows(); iSel++)
	{
		if (rstSelection.valByColName(iSel, "selNum") != "0")
		{
			//no cup custom
			cupCustomBtnAppearance[iSel] = "";
		}
		else
		{
			cupCustomBtnAppearance[iSel] = defCupAppearanceA +defCupAppearanceB;
			if (rstSelection.valByColName(iSel, "bCustomBtnCupAppeance") != "0")
			{				
				var UID = rstSelection.valByColName(iSel, "UID");
				rstCupAppearance = await db.q("SELECT AppearanceA,AppearanceB FROM cupcustombtn WHERE MMI='" +UID +"' AND HIS_ID=" +his_id);
				if (rstCupAppearance.getNumRows()> 0)
					cupCustomBtnAppearance[iSel] = rstCupAppearance.valByColName(0, "AppearanceA") +rstCupAppearance.valByColName(0, "AppearanceB");	
			}
		}
	}
console.timeEnd("translation 02");

	//testo di default dei btn optionA/B, in lingua di default
console.time("translation 03");	
	rstCupAppearance = await db.q("SELECT UID,Message FROM lang WHERE ISO='" +allLang[0] +"' AND What='-1' AND (UID='CUP_OPTAL' or UID='CUP_OPTAR' or UID='CUP_OPTBL' or UID='CUP_OPTBR')");
	var textOptionAL_defLang="";
	var textOptionAR_defLang="";
	var textOptionBL_defLang="";
	var textOptionBR_defLang="";
	for (var i=0; i<rstCupAppearance.getNumRows(); i++)
	{
		switch (rstCupAppearance.valByColName(i, "UID"))
		{
		case "CUP_OPTAL": textOptionAL_defLang=rstCupAppearance.valByColName(i, "Message"); break;
		case "CUP_OPTAR": textOptionAR_defLang=rstCupAppearance.valByColName(i, "Message"); break;
		case "CUP_OPTBL": textOptionBL_defLang=rstCupAppearance.valByColName(i, "Message"); break;
		case "CUP_OPTBR": textOptionBR_defLang=rstCupAppearance.valByColName(i, "Message"); break;
		}
	}
	rstCupAppearance = null;
console.timeEnd("translation 03");	
	

	//per ogni lingua...
console.time("translation per ogni lingua");
	var defaultSelName=[];
	var defaultSelInfo=[];
	var defaultNutriInfo=[];
	var defaultLearnMore="";

	for (var iLang=0; iLang<allLang.length; iLang++)
	{
		var iso = allLang[iLang];
console.time(iso);
		pleaseWait_addMessage("&nbsp;&nbsp;&nbsp;" +iso);
		
		//page learn more
		var bgImage="";
		for (var r=0; r<rstLearnMore.getNumRows(); r++)
		{
			if (iso == rstLearnMore.valByColName(r, "ISO"))
			{
				bgImage = rstLearnMore.valByColName(r, "bgImage");
				if (iLang==0)
					defaultLearnMore = bgImage;
				break;
			}
		}		
		
		if (bgImage=="" || bgImage=="NULL")
			bgImage=defaultLearnMore;
		var result = "var pageInfo_bg=\"" +bgImage +"\";";
		await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config/lang", iso +"_pageInfo.js");
		
		
console.time("  " +iso +"04");
		var stndByMsg = priv_buildConfigFile_translation_findISO_orDefault (rstStdbyMessage, iso, iLang, allLang[0]);
		var msgBtnStop = priv_buildConfigFile_translation_findISO_orDefault (rstBTN_STOP, iso, iLang, allLang[0]);
		var msgBtnStart = priv_buildConfigFile_translation_findISO_orDefault (rstBTN_START_SELECTION, iso, iLang, allLang[0]);
		var msgBtnStartNotAvail = priv_buildConfigFile_translation_findISO_orDefault (rstBTN_START_SELECTION_NOT_AVAIL, iso, iLang, allLang[0]);
		var msgBtnStartOffline = priv_buildConfigFile_translation_findISO_orDefault (rstBTN_START_OFFLINE, iso, iLang, allLang[0]);
		var msgBtnAbort = priv_buildConfigFile_translation_findISO_orDefault (rstBTN_ABORT, iso, iLang, allLang[0]);
		var msgLabPleaseScanQRCode = priv_buildConfigFile_translation_findISO_orDefault (rstLAB_PLEASE_SCAN_QRCODE, iso, iLang, allLang[0]);
		var msgLabCurrency = priv_buildConfigFile_translation_findISO_orDefault (rstLAB_CURRENCY_SIMBOL, iso, iLang, allLang[0]);
		var msgLabDrinkReady = priv_buildConfigFile_translation_findISO_orDefault (rstLAB_YOUR_DRINK_IS_READY, iso, iLang, allLang[0]);
		var msgLabDrinkBeingPrepared = priv_buildConfigFile_translation_findISO_orDefault (rstLAB_YOUR_DRINK_IS_BEING_PREPARED, iso, iLang, allLang[0])
console.timeEnd("  " +iso +"04");

		var result = "var rheaLang = {"
						+"BTN_STOP: \"" +msgBtnStop +"\""
						+",BTN_START_SELECTION: \"" +msgBtnStart +"\""
						+",BTN_START_SELECTION_NOT_AVAIL: \"" +msgBtnStartNotAvail +"\""
						+",BTN_START_OFFLINE: \"" +msgBtnStartOffline +"\""
						+",LAB_CURRENCY_SIMBOL: \"" +msgLabCurrency +"\""
						+",LAB_YOUR_DRINK_IS_READY: \"" +msgLabDrinkReady +"\""
						+",LAB_YOUR_DRINK_IS_BEING_PREPARED: \"" +msgLabDrinkBeingPrepared +"\""
						+",LAB_PAGE_STANDBY: \"" +stndByMsg +"\""
						+",BTN_ABORT: \"" +msgBtnAbort +"\""
						+",LAB_PLEASE_SCAN_QRCODE: \"" +msgLabPleaseScanQRCode +"\""
						+"};";
		var objectFooter = "var objFooter = {";
		
		for( var i=0; i<rstFOOTER_BTNS.getNumRows(); i++ ) {
			var _what = rstFOOTER_BTNS.valByColName(i, 'What');
			var _msg = rstFOOTER_BTNS.valByColName(i, 'Message');

			objectFooter += "\"" + _what + "\" : \"" + _msg + "\""

			if( i !== rstFOOTER_BTNS.getNumRows() - 1 ) {
				objectFooter += ','
			}
		}

		result += objectFooter + "};";
	
		//nomi delle selezioni per lingua
console.time("  " +iso +"05");
console.time("  " +iso +"05_rst");
		let rstTranslation = await db.q("SELECT What,Message FROM lang WHERE UID='MMI_NAME' AND ISO='" +iso +"'");
console.timeEnd("  " +iso +"05_rst");		
		var selectionNames = "";
		for (var iSel=0; iSel<rstSelection.getNumRows(); iSel++)
		{
			if (selectionNames!="")
				selectionNames +=",";
			
			var UID = rstSelection.valByColName(iSel, "UID");
			var s = priv_buildConfigFile_translation_findWHAT (rstTranslation, UID);
			
			//se vuoto, default alla traduzione del linguaggio di default
			if (iLang==0)
				defaultSelName[iSel] = s;
			else if (s=="" || s=="NULL")
				s = defaultSelName[iSel];
			
			selectionNames +="\"" +s +"\"";
		}
		selectionNames = "var rheaLang_mainMenuIconName=[" +selectionNames +"];";
		result += selectionNames;
		// Retrieves second name for beverage
		let rstSecondNameDescription = await db.q("SELECT What FROM pagemenu_mmi ORDER BY PROGR ASC");
		var secondName = '';

		for( var i=0; i<rstSecondNameDescription.getNumRows(); i++ ) {
			var _what = rstSecondNameDescription.valByColName(i, 'What');

			if( secondName ) { secondName += ','; }

			secondName +="\"" + _what +"\"";
		}

		secondName = "var rhea_mainMenuIconSecondName=[" + secondName +"];";
		result += secondName;
console.time("  " +iso +"05_saveas");
		await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config/lang", iso +".js");
console.timeEnd("  " +iso +"05_saveas");
console.timeEnd("  " +iso +"05");
		
		//decrizione delle selezioni per lingua
console.time("  " +iso +"06");		
		rstTranslation = await db.q("SELECT What,Message FROM lang WHERE UID='MMI_DESCR' AND ISO='" +iso +"'");
		var selectionDescr = "";
		for (var iSel=0; iSel<rstSelection.getNumRows(); iSel++)
		{
			if (selectionDescr!="")
				selectionDescr +=",";
			
			var UID = rstSelection.valByColName(iSel, "UID");
			var s = priv_buildConfigFile_translation_findWHAT (rstTranslation, UID);
			
			//se vuoto, default alla traduzione del linguaggio di default
			if (iLang==0)
				defaultSelInfo[iSel] = s;
			else if (s=="" || s=="NULL")
				s = defaultSelInfo[iSel];
			
			if (s=="NULL") s="";
			selectionDescr +="\"" +s +"\"";
		}
		selectionDescr = "var rheaMainMenuIconsInfo=[" +selectionDescr +"];";
		await buildConfigFile_saveAs (selectionDescr, rheaGetAbsolutePhysicalPath()+"/../config/lang", iso +"_mainMenuIconsInfo.js");		
console.timeEnd("  " +iso +"06");		
		
		//nutritional info per lingua
console.time("  " +iso +"07");		
		rstTranslation = await db.q("SELECT What,Message FROM lang WHERE UID='MMI_NUTRI' AND ISO='" +iso +"'");
		var nutritionalImage = "";
		for (var iSel=0; iSel<rstSelection.getNumRows(); iSel++)
		{
			if (nutritionalImage!="")
				nutritionalImage +=",";
			
			var UID = rstSelection.valByColName(iSel, "UID");
			var s = priv_buildConfigFile_translation_findWHAT (rstTranslation, UID);
			
			//se vuoto, default alla traduzione del linguaggio di default
			if (iLang==0)
				defaultNutriInfo[iSel] = s;
			else if (s=="" || s=="NULL")
				s = defaultNutriInfo[iSel];
			
			if (s=="NULL") s="";
			nutritionalImage +="\"" +s +"\"";
		}
		nutritionalImage = "var rheaLang_nutriInfo=[" +nutritionalImage +"];";
		await buildConfigFile_saveAs (nutritionalImage, rheaGetAbsolutePhysicalPath()+"/../config/lang", iso +"_nutriInfo.js");
console.timeEnd("  " +iso +"07");
		
		//cup customization option button appearance/text
console.time("  " +iso +"08");		
		var result = "";
		for (var iSel=0; iSel<rstSelection.getNumRows(); iSel++)
		{
			if (result!="")
				result +=",";

			if (rstSelection.valByColName(iSel, "selNum") != "0")
			{
				//no cup customization
				result +="\"\"";
			}
			else
			{
				result += "\"" +cupCustomBtnAppearance[iSel];
				
				var textOptionAL = textOptionAL_defLang;
				var textOptionAR = textOptionAR_defLang;
				var textOptionBL = textOptionBL_defLang;
				var textOptionBR = textOptionBR_defLang;				
				rstCupAppearance = await db.q("SELECT UID,Message FROM lang WHERE ISO='" +iso +"' AND What='-1' AND (UID='CUP_OPTAL' or UID='CUP_OPTAR' or UID='CUP_OPTBL' or UID='CUP_OPTBR') ORDER BY What DESC");
				for (var i=0; i<rstCupAppearance.getNumRows(); i++)
				{
					var v = rstCupAppearance.valByColName(i, "Message");
					if (v!="" && v!="NULL")
					{
						switch (rstCupAppearance.valByColName(i, "UID"))
						{
						case "CUP_OPTAL": textOptionAL=rstCupAppearance.valByColName(i, "Message"); break;
						case "CUP_OPTAR": textOptionAR=rstCupAppearance.valByColName(i, "Message"); break;
						case "CUP_OPTBL": textOptionBL=rstCupAppearance.valByColName(i, "Message"); break;
						case "CUP_OPTBR": textOptionBR=rstCupAppearance.valByColName(i, "Message"); break;
						}
					}
				}
				rstCupAppearance = null;
				
				//se la MMI in questione ha una sua ulteriore personalizzazione..
				if (rstSelection.valByColName(iSel, "bCustomBtnCupAppeance") != "0")
				{
					var UID = rstSelection.valByColName(iSel, "UID");
					
					rstCupAppearance = await db.q("SELECT UID,Message FROM lang WHERE ISO='" +allLang[0] +"' AND What='" +UID +"' AND (UID='CUP_OPTAL' or UID='CUP_OPTAR' or UID='CUP_OPTBL' or UID='CUP_OPTBR') ORDER BY What DESC");
					for (var i=0; i<rstCupAppearance.getNumRows(); i++)
					{
						var v = rstCupAppearance.valByColName(i, "Message");
						if (v!="" && v!="NULL")
						{
							switch (rstCupAppearance.valByColName(i, "UID"))
							{
							case "CUP_OPTAL": textOptionAL=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTAR": textOptionAR=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTBL": textOptionBL=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTBR": textOptionBR=rstCupAppearance.valByColName(i, "Message"); break;
							}
						}
					}
					rstCupAppearance = null;					
					
					
					rstCupAppearance = await db.q("SELECT UID,Message FROM lang WHERE ISO='" +iso +"' AND What='" +UID +"' AND (UID='CUP_OPTAL' or UID='CUP_OPTAR' or UID='CUP_OPTBL' or UID='CUP_OPTBR') ORDER BY What DESC");
					for (var i=0; i<rstCupAppearance.getNumRows(); i++)
					{
						var v = rstCupAppearance.valByColName(i, "Message");
						if (v!="" && v!="NULL")
						{
							switch (rstCupAppearance.valByColName(i, "UID"))
							{
							case "CUP_OPTAL": textOptionAL=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTAR": textOptionAR=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTBL": textOptionBL=rstCupAppearance.valByColName(i, "Message"); break;
							case "CUP_OPTBR": textOptionBR=rstCupAppearance.valByColName(i, "Message"); break;
							}
						}
					}
					rstCupAppearance = null;					
				}
				
				result+= "§" +textOptionAL +"§" +textOptionAR +"§" +textOptionBL +"§" +textOptionBR +"\"";
			}
		}
		result = "var rheaLang_cupAppearance=[" +result +"];";
console.time("  " +iso +"08_saveAs");
		await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config/lang", iso +"_cupAppearance.js");
console.timeEnd("  " +iso +"08_saveAs");
console.timeEnd("  " +iso +"08");
console.timeEnd(iso);
	}
console.timeEnd("translation per ogni lingua");	
}

function priv_buildConfigFile_translation_findWHAT (rst, what)
{
	for (var r=0; r<rst.getNumRows(); r++)
	{
		if (rst.valByColName(r,"What") == what)
		{
			var ret = rst.valByColName(r,"Message").replace(/"/g, '&#34;');
			return ret.replace(/\n/g, '<br>');
		}
	}
	return "";
}

function priv_buildConfigFile_translation_findISO (rst, iso)
{
	for (var r=0; r<rst.getNumRows(); r++)
	{
		if (rst.valByColName(r,"ISO") == iso)
		{
			var ret = rst.valByColName(r,"Message").replace(/"/g, '&#34;');
			return ret.replace(/\n/g, '<br>');
		}
	}
	return "";
}

function priv_buildConfigFile_translation_findISO_orDefault (rst, iso, iLang, defaultLang)
{
	var retMsg = priv_buildConfigFile_translation_findISO(rst, iso);
	if ((retMsg=="" || retMsg=="NULL") && iLang>0)
		retMsg = priv_buildConfigFile_translation_findISO(rst, defaultLang);	
	return retMsg;
}


/*********************************************************
 * crea il file config/lang/menuProg.txt per ogni lingua supportata
 */
async function buildConfigFile_menuProg (db, his_id)
{
	var N_SELECTIONS=48;
	var info = [];
	for (var iSel=1; iSel<=N_SELECTIONS; iSel++)
		info[iSel] = {"UID":0, "shortcut":"xxx", "name":""}


	let rstSelection = await db.q("SELECT UID,selNum,optionAEnabled,optionBEnabled,allowedCupSize,linkedSelection FROM pagemenu_mmi WHERE HIS_ID=" +his_id +" ORDER BY selNum DESC");

	//prima quelle facili, ovvero quello senza cup customization
	for (var i=0; i<rstSelection.getNumRows(); i++)
	{
		var selNum = parseInt(rstSelection.valByColName(i, "selNum"));
		if (selNum == 0)
			break;
		info[selNum].UID = parseInt(rstSelection.valByColName(i, "UID"));
		info[selNum].shortcut = "xx";
		
		var cupSize = rstSelection.valByColName(i, "allowedCupSize");
		if (cupSize.substr(0,1) == "1")
			info[selNum].shortcut += "S";
		else if (cupSize.substr(1,1) == "1")
			info[selNum].shortcut += "M";
		else
			info[selNum].shortcut += "L";
	}
	
	//ora quelle con cup-custom
	for (var iSel=1; iSel<=N_SELECTIONS; iSel++)
	{
		if (info[iSel].UID != 0)
			continue;
		
		//devo cercare se esiste una MMI che include la selezione iSel
		for (var i=0; i<rstSelection.getNumRows(); i++)
		{
			var linkedSelection = rstSelection.valByColName(i, "linkedSelection").split(",");
			for (var i2=0; i2<linkedSelection.length; i2++)
			{
				/**************************************************************
				//      0  1  2  3           4  5  6  7            8  9  10 11
				//small LL LR RL RR   medium LL LR RL RR     large LL LR RL RR
				*/
				if (parseInt(linkedSelection[i2]) == iSel)
				{
					info[iSel].UID = parseInt(rstSelection.valByColName(i, "UID"));
					
					switch (i2)
					{
						case 0: info[iSel].shortcut = "LLS"; break;
						case 1: info[iSel].shortcut = "LRS"; break;
						case 2: info[iSel].shortcut = "RLS"; break;
						case 3: info[iSel].shortcut = "RRS"; break;
						
						case 4: info[iSel].shortcut = "LLM"; break;
						case 5: info[iSel].shortcut = "LRM"; break;
						case 6: info[iSel].shortcut = "RLM"; break;
						case 7: info[iSel].shortcut = "RRM"; break;
						
						case 8: info[iSel].shortcut = "LLL"; break;
						case 9: info[iSel].shortcut = "LRL"; break;
						case 10: info[iSel].shortcut = "RLL"; break;
						case 11: info[iSel].shortcut = "RRL"; break;
					}
					
					
					if (rstSelection.valByColName(i, "optionAEnabled") == "1")
					{
						if (rstSelection.valByColName(i, "optionBEnabled") == "1")
						{
							//A & B enabled
						}
						else
						{
							//A enabled & B not enabled
							info[iSel].shortcut = setCharAt(info[iSel].shortcut, 1, "x");
						}
					}
					else
					{
						if (rstSelection.valByColName(i, "optionBEnabled") == "1")
						{
							//A not enabled & B enabled
							info[iSel].shortcut = setCharAt(info[iSel].shortcut, 0, "x");
						}
						else
						{
							//A not enabled & B not enabled
							info[iSel].shortcut = setCharAt(info[iSel].shortcut, 0, "x");
							info[iSel].shortcut = setCharAt(info[iSel].shortcut, 1, "x");
						}
					}
					break;
				}
			}
		}
	}
	

	//lingue
	let rst = await db.q("SELECT * FROM generalset WHERE HIS_ID=" +his_id);
	var allLangString = rst.valByColName(0, "allowedLang");
	var allLang = allLangString.split(",");
	var defaultLang = rst.valByColName(0, "defaultLang");

	var iso = defaultLang;
	for (var iSel=1; iSel<=N_SELECTIONS; iSel++)
	{
		if (info[iSel].UID == 0)
			continue;
		
		let rstTranslation = await db.q("SELECT Message FROM lang WHERE UID='MMI_NAME' AND What=" +info[iSel].UID +" AND ISO='" +iso +"'");
		info[iSel].name = rstTranslation.valByColName(0, "Message");
	}

	var result = info[1].shortcut +"|" +info[1].name;
	for (var iSel=2; iSel<=N_SELECTIONS; iSel++)
		result += "|" +info[iSel].shortcut +"|" +info[iSel].name;
	await buildConfigFile_saveAs (result, rheaGetAbsolutePhysicalPath()+"/../config/lang", "menuProg.txt");			
	//console.log (info);
}
