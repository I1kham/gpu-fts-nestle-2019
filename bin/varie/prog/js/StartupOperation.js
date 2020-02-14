var cpuLangName1 = "";
var cpuLangName2 = "";
var bGotoHMIAfterLavaggio = 0;

function startupOperation()
{
	syncDA3IfNeeded();
}

function syncDA3IfNeeded ()
{
	pleaseWait_freeText_appendText("da3 sync...");
	rhea.sendSyncDA3();
	setTimeout (function(){ syncDA3IfNeeded_onProgress();}, 600);
}

function syncDA3IfNeeded_onProgress ()
{
	if (currentCPUStatusID != -1 && currentCPUStatusID != 105)
	{
		pleaseWait_freeText_appendText("<br>");
		priv_askCPULangNames();
	}
	else
	{
		setTimeout (function(){ syncDA3IfNeeded_onProgress();}, 800);
		pleaseWait_freeText_appendText(".");
	}
}

function priv_askCPULangNames()
{
	pleaseWait_freeText_appendText("getting CPU lang names...<br>");
	rhea.ajax ("getCPULangNames", "").then( function(result)
	{
		var e = result.split("#");
		cpuLangName1 = e[0];
		cpuLangName2 = e[1];
		priv_askMachineTypeAndModel();
	})
	.catch( function(result)
	{
		console.log ("err[" +result +"]");
		cpuLangName1 = "lang 1";
		cpuLangName1 = "lang 2";
		setTimeout(function() { priv_askCPULangNames(); }, 200);
	});	
}

function priv_askMachineTypeAndModel()
{
	pleaseWait_freeText_appendText("getting machine type and model...<br>");
	rhea.ajax ("getMachineTypeAndModel", "").then( function(result)
	{
		var obj = JSON.parse(result);
		
		//carico il da3
		pleaseWait_freeText_appendText("loading da3...<br>");
		da3 = new DA3(obj.mType, obj.mModel, obj.isInduzione);
		da3.load();
	})
	.catch( function(result)
	{
		setTimeout(function() { priv_askMachineTypeAndModel(); }, 200);
	});	
}

//***************** chiamata al termine del caricamento del da3
function onDA3Loaded(bResult)
{
	if (bResult == 0)
	{
		setTimeout ( function() { window.location = "index.html"; }, 1000);
		return;
	}

	//console.log ("onDA3Loaded:: num macine[" +da3.getNumMacine() +"]");
	guiInfo = new GUIInfo();
	guiInfo.load();
}

//***************** chiamata al termine delle gui info
function onGUIInfoLoaded()
{
	pageProductQty_build();
	pageSelections_build();
	pageSingleSelection_build();
	pageTemperature_build();
	pleaseWait_hide();
	
	//chiedo se il manuale e' installato, nel qual caso abilito il relativo bottone
	rhea.ajax ("isManInstalled", "").then( function(result)
	{
		if (result!="KO")
		{
			rheaShowElem(rheaGetElemByID("divBtnManual"));
			manualFolderName = result;
		}
	})
	.catch( function(result)
	{
	});
	
	
	//abilito il menu milker se macchine==espresso
	if (da3.isEspresso())
	{
		rheaRemoveClassToElem(rheaGetElemByID("pageMainMenu_btnMilker"), "UIdisabled");
		rheaRemoveClassToElem(rheaGetElemByID("pageCleaning_btnCleanMilker"), "UIdisabled");
	}
	
	//da ora in poi, il currentTask.onTimer() viene chiamata una volta al secondo
	setInterval (function() { timeNowMSec+=1000; currentTask.onTimer(timeNowMSec); }, 1000)
	
	//di default mostro la pagina pageMainMenu. Se però in GET c'è specificata un'altra pagina..
	switch (rheaGetURLParamOrDefault("page", "pageMainMenu"))
	{
		case "pageMainMenu": 		pageMainMenu_show(); break;
		case "pageCleaning": 		pageCleaning_show(); break;
		case "pageMaintenance": 	pageMaintenance_show(); break;
		case "pageMiscellaneous": 	pageMiscellaneous_show(); break;
		case "pageProductQty": 		pageProductQty_show(); break;
		case "pageMilker": 			pageMilker_show(); break;
		
		case "pageCleaningSanitario": 
			//start del lavaggio sanitario e, alla fine, goto back to HMI
			pageCleaning_show(); 
			pageCleaning_startLavSanitario(8,0); 
			bGotoHMIAfterLavaggio = 1;
			break;

		case "pageCleaningMilker": 
			//start del lavaggio milker e, alla fine, goto back to HMI
			pageCleaning_show(); 
			
			//prima di poter partire col lavaggio sanitario del milker, bisogna che la CPU non sia ancora in stato INI_CHECK
			bGotoHMIAfterLavaggio = 1;
			pleaseWait_show(); 
			pleaseWait_freeText_show();
			pleaseWait_freeText_setText("MILK MODULE RINSING<br>Waiting for machine to be ready");
			setTimeout (waitCPUNoMoreInINI_CHECK_status_thenStartCleanMilker(3), 100);
			break;
	}	
}
	
//**** attende che lo stato di CPU diventi == READY e poi parte con il lavaggio sanitario
function waitCPUNoMoreInINI_CHECK_status_thenStartCleanMilker(howManyTimesLeft)
{
	pleaseWait_freeText_appendText(".");
	if (currentCPUStatusID != 2) //2 == READY
	{
		setTimeout (function() { waitCPUNoMoreInINI_CHECK_status_thenStartCleanMilker(3); }, 1000);
		return;
	}
	
	howManyTimesLeft--;
	if (howManyTimesLeft == 0)
	{
		pleaseWait_freeText_appendText ("<br>Cleaning is starting");
		pageCleaning_startLavSanitario(5,1); 
	}
	else
		setTimeout (function() { waitCPUNoMoreInINI_CHECK_status_thenStartCleanMilker(howManyTimesLeft); }, 1000);
}