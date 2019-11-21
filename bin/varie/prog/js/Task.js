/********************************************************
 * Task
 *
 * Questo è il template delle classi "task".
 * Tutte le classi "derivate", devono implementare i metodi "on"
 */
function TaskVoid()																{}
TaskVoid.prototype.onTimer 				= function(timeNowMsec)					{}
TaskVoid.prototype.onEvent_cpuStatus 	= function(statusID, statusStr)			{}
TaskVoid.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskVoid.prototype.onFreeBtn1Clicked	= function(ev)							{}
TaskVoid.prototype.onFreeBtn2Clicked	= function(ev)							{}
TaskVoid.prototype.onExit				= function(bSave)						{ return bSave; }



/**********************************************************
 * TaskCleaning
 */
function TaskCleaning(bIsSanWashing)
{
	this.timeStarted = 0;
	this.cpuStatus = 0;
	this.bIsSanWashing = bIsSanWashing;
	this.fase = 0;
	this.btn1 = 0;
	this.btn2 = 0;
	this.nextTimeSanWashStatusCheckMSec = 0;
}
TaskCleaning.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	if (this.bIsSanWashing)
	{
		this.priv_handleSanWashing(timeElapsedMSec);
		return;
	}
	
	if (timeElapsedMSec < 2000)
		return;
	if (this.cpuStatus != 7) //7==manual washing
		pageCleaning_onFinished();
}

TaskCleaning.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskCleaning.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }

TaskCleaning.prototype.onFreeBtn1Clicked	= function(ev)						{ rhea.sendButtonPress(this.btn1); pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); }
TaskCleaning.prototype.onFreeBtn2Clicked	= function(ev)						{ rhea.sendButtonPress(this.btn2); pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); }
TaskCleaning.prototype.onExit				= function(bSave)					{ return bSave; }

TaskCleaning.prototype.priv_handleSanWashing = function (timeElapsedMSec)
{
	//termino quando lo stato della CPU diventa != da SAN_WASHING
	if (timeElapsedMSec > 3000 && this.cpuStatus != 20) //20==sanitary washing
	{
		pageCleaning_onFinished();
		return;
	}

	//ogni tot mando una richiesta per conoscere lo stato attuale del lavaggio
	if (timeElapsedMSec < this.nextTimeSanWashStatusCheckMSec)
		return;
	this.nextTimeSanWashStatusCheckMSec += 2000;

	//periodicamente richiedo lo stato del lavaggio
	var me = this;
	rhea.ajax ("sanWashStatus", "")
		.then( function(result) 
		{
			var obj = JSON.parse(result);
			//console.log ("SAN WASH response: fase[" +obj.fase +"] b1[" +obj.btn1 +"] b2[" +obj.btn2 +"]");
			me.fase = parseInt(obj.fase);
			me.btn1 = parseInt(obj.btn1);
			me.btn2 = parseInt(obj.btn2);
			switch (me.fase)
			{
				case 0: pleaseWait_freeText_setText("Health Cleaning is not started (or ended)"); break;
				case 1: pleaseWait_freeText_setText("Health Cleaning is started"); break;
				case 2: pleaseWait_freeText_setText("brewer placed"); break;
				case 3: pleaseWait_freeText_setText("wait for tablet"); break;
				case 4: pleaseWait_freeText_setText("infusion"); break;
				case 5: pleaseWait_freeText_setText("brewer cleaning cycles 1"); break;
				case 6: pleaseWait_freeText_setText("brewer cleaning cycles 2"); break;
				case 7: pleaseWait_freeText_setText("brewer cleaning cycles 3"); break;
				case 8: pleaseWait_freeText_setText("brewer cleaning cycles 4"); break;
				case 9: pleaseWait_freeText_setText("brewer cleaning cycles 5"); break;
				case 10: pleaseWait_freeText_setText("brewer cleaning cycles 6"); break;
				case 11: pleaseWait_freeText_setText("mixer 1 cleaning"); break;
				case 12: pleaseWait_freeText_setText("mixer 2 cleaning"); break;
				case 13: pleaseWait_freeText_setText("mixer 3 cleaning"); break;
				case 14: pleaseWait_freeText_setText("mixer 4 cleaning"); break;
				default: pleaseWait_freeText_setText(""); break;
			}
			pleaseWait_freeText_show();
			
			if (me.btn1 == 0)
				pleaseWait_btn1_hide();
			else
			{
				pleaseWait_btn1_setText ("BUTTON " +me.btn1);
				pleaseWait_btn1_show();	
			}
			
			if (me.btn2 == 0)
				pleaseWait_btn2_hide();
			else
			{
				pleaseWait_btn2_setText ("BUTTON " +me.btn2);
				pleaseWait_btn2_show();	
			}			
		})
		.catch( function(result)
		{
			//console.log ("SANWASH: error[" +result +"]");
			pleaseWait_btn1_hide();
			pleaseWait_btn2_hide();
		});	
	
}



/**********************************************************
 * TaskCalibMotor
 */
function TaskCalibMotor()
{
	this.timeStarted = 0;
	this.what = 0;  //0==nulla, 1=calib motore prodott, 2=calib macina
	this.fase = 0;
	this.value = 0;
}
TaskCalibMotor.prototype.startMotorCalib = function (motorIN)
{
	this.timeStarted = 0;
	this.fase = 0;
	this.motor = motorIN;
	this.value = 0;
	this.impulsi = 0;
	
	this.what = 0;
	if (motorIN == 11 || motorIN == 12)
		this.what = 2;
	else
		this.what = 1;
	
}

TaskCalibMotor.prototype.onEvent_cpuStatus 	= function(statusID, statusStr)			{}
TaskCalibMotor.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskCalibMotor.prototype.onFreeBtn2Clicked	= function(ev)	{ }
TaskCalibMotor.prototype.onExit				= function(bSave)						{ return bSave; }
TaskCalibMotor.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;

	switch (this.what)
	{
		case 1: this.priv_handleCalibProdotto(timeElapsedMSec); break;
		case 2: this.priv_handleCalibMacina(timeElapsedMSec); break;
	}
}

TaskCalibMotor.prototype.onFreeBtn1Clicked	= function(ev)
{ 
	switch (this.what)
	{
	case 1:
		if (this.fase == 30)	{ pleaseWait_btn1_hide(); this.fase = 40; }
		break;
		
	case 2:
		if (this.fase == 1)		{ pleaseWait_btn1_hide(); this.fase = 2; }
		else if (this.fase==21) { pleaseWait_btn1_hide();; this.fase=30; }
		else if (this.fase==41) { pleaseWait_btn1_hide();; this.fase=50; }
		break;		
	}
}

TaskCalibMotor.prototype.priv_handleCalibProdotto = function (timeElapsedMSec)
{
	var TIME_ATTIVAZIONE_dSEC = 30;
	
	var me = this;
	//console.log ("TaskCalibMotor::fase[" +me.fase +"]");
	switch (this.fase)
	{
	case 0:
		me.fase = 10;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("Please wait while motor is running");
		rhea.ajax ("runMotor", { "m":me.motor, "d":TIME_ATTIVAZIONE_dSEC, "n":2, "p":10}).then( function(result)
		{
			setTimeout ( function() { me.fase=20; }, TIME_ATTIVAZIONE_dSEC*2*100 - 1000);
		})
		.catch( function(result)
		{
			//console.log ("TaskCalibMotor:: error fase 0 [" +result +"]");
			me.fase = 200;
		});					
	break;
		
	case 10:
		break;
		
	case 20:
		me.fase = 30;
		pleaseWait_calibration_setText("Please enter the quantity, then press CONTINUE");
		pleaseWait_calibration_num_setValue(0);
		pleaseWait_calibration_num_show();
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		break;
		
	case 30:
		break;
		
	case 40:
		pleaseWait_calibration_setText("Storing value...");
		me.value = pleaseWait_calibration_num_getValue();
		me.gsec = parseInt( Math.round(me.value / (TIME_ATTIVAZIONE_dSEC*0.2)) );
		pleaseWait_calibration_num_hide();
		
		//console.log ("TaskCalibMotor::[40] motor[" +me.motor +"] value[" +me.value +"] g/sec[" +me.gsec +"]");
		
		rhea.ajax ("setFattoreCalib", { "m":me.motor, "v":me.gsec}).then( function(result)
		{
			me.fase = 199;
		})
		.catch( function(result)
		{
			//console.log ("TaskCalibMotor:: error fase 40 [" +result +"]");
			me.fase = 200;
		});		
		break;

		
	case 199:
		//console.log ("TaskCalibMotor::[199] motor[" +me.motor +"] value[" +me.value +"] g/sec[" +me.gsec +"]");
		da3.setCalibFactorGSec(me.motor, me.gsec);
		var v = helper_intToFixedOnePointDecimale( da3.getCalibFactorGSec(me.motor) );
		rheaSetDivHTMLByName("pageCalibration_m" +me.motor, v +"&nbsp;gr/sec");
		me.fase = 200;
		break;
		
	case 200:
		me.what = 0;
		pleaseWait_btn1_hide();
		pleaseWait_calibration_hide();
		pageCalibration_onFinish();
		break;
	}
}

TaskCalibMotor.prototype.priv_handleCalibMacina = function (timeElapsedMSec)
{
	var TIME_ATTIVAZIONE_dSEC = 60;
	
	var me = this;
	//console.log ("TaskCalibMotor::fase[" +me.fase +"]");
	switch (this.fase)
	{
	case 0:
		me.fase = 1;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("Please remove the brewer, then press CONTINUE");
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		break;
		
	case 1:	//attendo btn CONTINUE
		break;
		
	case 2: //verifico che il gruppo sia scollegato, altrimenti goto 0
		rhea.ajax ("getGroupState", "").then( function(result)
		{
			//console.log ("TaskCalibMotor, grpState[" +result +"]");
			if (result=="0")
				me.fase = 10;
			else
				me.fase = 0;
		})
		.catch( function(result)
		{
			me.fase = 0;
		});			
		me.fase = 3;
		break;
		
	case 3:	//attendo risposta CPU
		break;
		
	case 10:  //attivo le macinate
		me.fase = 11;
		pleaseWait_calibration_setText("Please wait while motor is running");
		rhea.ajax ("runMotor", { "m":me.motor, "d":TIME_ATTIVAZIONE_dSEC, "n":2, "p":10}).then( function(result)
		{
			setTimeout ( function() { me.fase=20; }, TIME_ATTIVAZIONE_dSEC*2*100 - 1000);
		})
		.catch( function(result)
		{
			me.fase = 200;
		});					
	break;
		
	case 11: //attendo fine macinate
		break;
		
	case 20:
		me.fase = 21;
		pleaseWait_calibration_setText("Please enter the quantity, then press CONTINUE");
		pleaseWait_calibration_num_setValue(0);
		pleaseWait_calibration_num_show();
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		break;
		
	case 21: //attendo pressione di continue
		break;
		
	case 30:
		pleaseWait_calibration_setText("Storing value...");
		me.value = pleaseWait_calibration_num_getValue();
		me.gsec = parseInt( Math.round(me.value / (TIME_ATTIVAZIONE_dSEC*0.2)) );
		pleaseWait_calibration_num_hide();
		
		rhea.ajax ("setFattoreCalib", { "m":me.motor, "v":me.gsec}).then( function(result)
		{
			me.fase = 40;
		})
		.catch( function(result)
		{
			me.fase = 200;
		});		
		break;
		
	case 40: //chiedo di rimettere a posto il gruppo
		pleaseWait_calibration_setText("Place the brewer into position, then press CONTINUE");
		pleaseWait_btn1_show();
		me.fase = 41;
		break;
		
	case 41:
		break;
		
	case 50: //verifico che il gruppo sia collegato, altrimenti goto 40
		rhea.ajax ("getGroupState", "").then( function(result)
		{
			//console.log ("TaskCalibMotor, grpState[" +result +"]");
			if (result=="1")
				me.fase = 60;
			else
				me.fase = 40;
		})
		.catch( function(result)
		{
			me.fase = 40;
		});			
		me.fase = 51;
		break;
		
	case 51://attendo risposta CPU
		break;
		
	case 60: //gruppo è stato ricollegato, procedo con il calcolo impulsi
		me.fase = 65;
		pleaseWait_calibration_setText("Impulse calculation in progress, please wait");
		rhea.ajax ("startImpulseCalc", { "m":me.motor, "v":me.value}).then( function(result)
		{
			//me.fase = 70;
			setTimeout ( function() { me.fase = 70; }, 3000);
		})
		.catch( function(result)
		{
			me.fase = 60;
		});			
		
	case 65: //attendo risposta CPU
		break;
		
	case 70: //cpu sta facendo i conti degli impulsi, mando query per sapere come sta
		me.fase = 71;
		rhea.ajax ("queryImpulseCalcStatus", "").then( function(result)
		{
			//console.log ("queryImpulseCalc::result[" +result +"]");
			var obj = JSON.parse(result);
			if (parseInt(obj.v) > 0)
			{
				me.impulsi= parseInt(obj.v);
				me.fase = 190;
			}
			else
				me.fase = 70;
		})
		.catch( function(result)
		{
			me.fase = 70;
		});			
		break;
		
	case 71:
		break;
		
		
	case 190: //devo memorizzare gli impulsi ricevuti nel da3??
		pleaseWait_calibration_setText("Impulse: " +me.impulsi);
		da3.setImpulsi(11, me.impulsi);
		me.fase = 191;
		break;
		
	case 191: me.fase++; break;
	case 192: me.fase++; break;
	case 193: me.fase++; break;
	case 194: me.fase++; break;
	case 195: me.fase=200; break;
		
	case 200:
		me.what = 0;
		pleaseWait_btn1_hide();
		pleaseWait_calibration_hide();
		pageCalibration_onFinish();
		break;
	}
}



/**********************************************************
 * TaskTestSelezione
 */
function TaskTestSelezione(iAttuatore)
{
	this.timeStarted = 0;
	this.iAttuatore = iAttuatore;
	this.cpuStatus = 0;
}
TaskTestSelezione.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	if (timeElapsedMSec < 2000)
		return;
	if (this.cpuStatus != 21 && this.cpuStatus != 101) //21==eVMCState_TEST_ATTUATORE_SELEZIONE
	{
		console.log (this.cpuStatus);
		pageSingleSelection_test_onFinish();
	}
}

TaskTestSelezione.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskTestSelezione.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }

TaskTestSelezione.prototype.onFreeBtn1Clicked	= function(ev)						{}
TaskTestSelezione.prototype.onFreeBtn2Clicked	= function(ev)						{}
TaskTestSelezione.prototype.onExit				= function(bSave)					{ return bSave; }


/**********************************************************
 * TaskDevices
 */
function TaskDevices()
{
	this.fase = 0;
	this.firstTimeMacina1 = 1;
	this.firstTimeMacina2 = 1;
}
TaskDevices.prototype.onTimer = function (timeNowMsec)
{
	var me = this;
	if (this.fase == 0)
	{
		this.fase = 1;
		rhea.ajax ("getPosMacina", {"m":1}).then( function(result)
		{
			var obj = JSON.parse(result);
			rheaSetDivHTMLByName("pageDevices_vg1", obj.v);
			if (me.firstTimeMacina1 == 1)
			{
				me.firstTimeMacina1 = 0;
				ui.getWindowByID("pageDevices").getChildByID("pageDevices_vg1_target").setValue(obj.v)
			}
		})
		.catch( function(result)
		{
		});			
		return;
	}
	else
	{
		this.fase = 0;
		rhea.ajax ("getPosMacina", {"m":2}).then( function(result)
		{
			var obj = JSON.parse(result);
			rheaSetDivHTMLByName("pageDevices_vg2", obj.v);
			if (me.firstTimeMacina2 == 1)
			{
				me.firstTimeMacina2 = 0;
				ui.getWindowByID("pageDevices").getChildByID("pageDevices_vg2_target").setValue(obj.v)
			}
		})
		.catch( function(result)
		{
		});			
		return;		
	}

}

TaskDevices.prototype.onEvent_cpuStatus  = function(statusID, statusStr)	{ }
TaskDevices.prototype.onEvent_cpuMessage = function(msg, importanceLevel)	{ rheaSetDivHTMLByName("footer_C", msg); }
TaskDevices.prototype.onFreeBtn1Clicked	 = function(ev)						{}
TaskDevices.prototype.onFreeBtn2Clicked	 = function(ev)						{}
TaskDevices.prototype.onExit			 = function(bSave)					{ return bSave; }