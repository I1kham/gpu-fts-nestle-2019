/*
	La pagina "please wait" consiste di una pagina in sovraimpressione composta da:
		- un header diviso in 2 parti (header_left e header_right)
		- una zona centrale (generalmente dove viene visualizzata la rotella e/oi il freetext)
		- 1 bottone in basso a sinistra (btn2)
		- 1 bottone in basso a destra	(btn1)
	
	E' possibile scrivere del testo nell'header usando le funzioni
		pleaseWait_header_setTextL()
		pleaseWait_header_setTextR()
		
	La zona centrale, può ospitare la classica rotella di "attendere prego", visualizzabile tramite le
		pleaseWait_rotella_show()
		pleaseWait_rotella_hide()
		
	La zona centrale può ospitare un ulteriore zona di "free text" che va in sovrapposizione alla eventuale rotella di attendere prego
		pleaseWait_freeText_show()
		pleaseWait_freeText_hide()
		pleaseWait_freeText_setText()
		pleaseWait_freeText_appendText
*/
function pleaseWait_setup()
{
	document.getElementById("pagePleaseWait_freeBtn1").addEventListener		("mousedown", function(ev) 	{ currentTask.onFreeBtn1Clicked(ev); } , true);
	document.getElementById("pagePleaseWait_freeBtn2").addEventListener		("mousedown", function(ev) 	{ currentTask.onFreeBtn2Clicked(ev); } , true);
	document.getElementById("pagePleaseWait_freeBtnTrick").addEventListener	("mousedown", function(ev) 	{ currentTask.onFreeBtnTrickClicked(ev); } , true);
}

//************* show/hide della pagina pleaseWait 
function pleaseWait_show()				
{ 
	pleaseWait_header_setTextL(""); 	pleaseWait_header_setTextR(""); 
	
	pleaseWait_btn1_hide();	pleaseWait_btn2_hide();	pleaseWait_btnTrick_hide();
	
	pleaseWait_freeText_setText(""); pleaseWait_freeText_hide();
	
	pleaseWait_rotella_show();
	rheaShowElem(rheaGetElemByID("pagePleaseWait")); 
}
function pleaseWait_hide()								{ rheaHideElem(rheaGetElemByID("pagePleaseWait")); }

//******** header
function pleaseWait_header_setTextL(s)					{ rheaSetDivHTMLByName("pagePleaseWait_header_left", s); }
function pleaseWait_header_setTextR(s)					{ rheaSetDivHTMLByName("pagePleaseWait_header_right", s); }

//********* rotella
function pleaseWait_rotella_show()						{ rheaShowElem(rheaGetElemByID("pagePleaseWait_wait")); }
function pleaseWait_rotella_hide()						{ rheaHideElem(rheaGetElemByID("pagePleaseWait_wait")); }

//******** bottoni nel footer
function pleaseWait_btn1_show()							{ rheaShowElem(rheaGetElemByID("pagePleaseWait_freeBtn1")); }
function pleaseWait_btn1_hide()							{ rheaHideElem(rheaGetElemByID("pagePleaseWait_freeBtn1")); }
function pleaseWait_btn1_setText(s)						{ rheaSetDivHTMLByName("bntFree1Text", s); }

function pleaseWait_btn2_show()							{ rheaShowElem(rheaGetElemByID("pagePleaseWait_freeBtn2")); }
function pleaseWait_btn2_hide()							{ rheaHideElem(rheaGetElemByID("pagePleaseWait_freeBtn2")); }
function pleaseWait_btn2_setText(s)						{ rheaSetDivHTMLByName("bntFree2Text", s); }

//******** bottone invisibile che si posiziona nell'angolo in alto a dx della zona "freetext".
function pleaseWait_btnTrick_show()						{ rheaShowElem(rheaGetElemByID("pagePleaseWait_freeBtnTrick")); }
function pleaseWait_btnTrick_hide()						{ rheaHideElem(rheaGetElemByID("pagePleaseWait_freeBtnTrick")); }

//********* free text
function pleaseWait_freeText_show()						{ rheaShowElem(rheaGetElemByID("pagePleaseWait_freeText")); }
function pleaseWait_freeText_hide()						{ rheaHideElem(rheaGetElemByID("pagePleaseWait_freeText")); }
function pleaseWait_freeText_setText(s)					{ rheaSetDivHTMLByName("pagePleaseWait_freeText", s); }
function pleaseWait_freeText_appendText(s)				{ var d=rheaGetElemByID("pagePleaseWait_freeText"); d.innerHTML += s; }


//********* usati durante la calibrazione della macina
function pleaseWait_calibration_show()					{ pleaseWait_calibration_num_hide(); pleaseWait_calibration_varigrind_hide(); rheaShowElem(rheaGetElemByID("pagePleaseWait_calibration")); }
function pleaseWait_calibration_hide()					{ pleaseWait_calibration_num_hide(); pleaseWait_calibration_varigrind_hide(); rheaHideElem(rheaGetElemByID("pagePleaseWait_calibration")); }
function pleaseWait_calibration_num_show()				{ rheaShowElem(rheaGetElemByID("pagePleaseWait_calibration_num")); }
function pleaseWait_calibration_num_hide()				{ rheaHideElem(rheaGetElemByID("pagePleaseWait_calibration_num")); }
function pleaseWait_calibration_num_setValue (v)		{ uiStandAloneNum.setValue(parseInt(v)); }
function pleaseWait_calibration_num_getValue()			{ return uiStandAloneNum.getValue(); }
function pleaseWait_calibration_setText(s)				{ rheaSetDivHTMLByName("pagePleaseWait_calibration_text", s); }
function pleaseWait_calibration_varigrind_show()		{ rheaShowElem(rheaGetElemByID("pagePleaseWait_calibration_1")); }
function pleaseWait_calibration_varigrind_hide()		{ rheaHideElem(rheaGetElemByID("pagePleaseWait_calibration_1")); }
function pleaseWait_calibration_motor_show()			{ rheaShowElem(rheaGetElemByID("pagePleaseWait_calibration_2")); }
function pleaseWait_calibration_motor_hide()			{ rheaHideElem(rheaGetElemByID("pagePleaseWait_calibration_2")); }