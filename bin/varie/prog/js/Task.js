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
 * TaskTemperature
 */
function TaskTemperature()																{ this.nextTimeCheckMSec=0;}
TaskTemperature.prototype.onEvent_cpuStatus 	= function(statusID, statusStr)			{}
TaskTemperature.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskTemperature.prototype.onFreeBtn1Clicked	= function(ev)								{}
TaskTemperature.prototype.onFreeBtn2Clicked	= function(ev)								{}
TaskTemperature.prototype.onExit				= function(bSave)						{ return bSave; }
TaskTemperature.prototype.onTimer 				= function(timeNowMsec)
{
	if (timeNowMsec >= this.nextTimeCheckMSec)
	{
		this.nextTimeCheckMSec = timeNowMsec + 1000;
		rhea.ajax ("getVandT", "").then( function(result)
		{
			var data = JSON.parse(result);
			rheaSetDivHTMLByName ("pageTemperature_live", pageMaintenance_formatHTMLForTemperature(data));
		})
		.catch( function(result)
		{
		});		
	}
}

/**********************************************************
 * TaskCleaning
 */
function TaskCleaning (whichWashIN)
{
	this.timeStarted = 0;
	this.cpuStatus = 0;
	this.whichWash = whichWashIN;
	this.fase = 0;
	this.btn1 = 0;
	this.btn2 = 0;
	this.nextTimeSanWashStatusCheckMSec = 0;
	
	rhea.sendGetCPUStatus();
}
TaskCleaning.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	if (this.whichWash == 8)
		this.priv_handleSanWashing(timeElapsedMSec);
	else if (this.whichWash == 5)
		this.priv_handleMilkWashing(timeElapsedMSec);
	else
	{
		if (timeElapsedMSec < 2000)
			return;
		if (this.cpuStatus != 7) //7==manual washing
			pageCleaning_onFinished();
	}
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

TaskCleaning.prototype.priv_handleMilkWashing = function (timeElapsedMSec)
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
				case 1: pleaseWait_freeText_setText("Milker Cleaning is started"); break;
				case 2: pleaseWait_freeText_setText("Warning for cleaner"); break;
				case 3: pleaseWait_freeText_setText("Wait for confirm"); break;
				case 4: pleaseWait_freeText_setText("It is doing cleaner cycles (12)"); break;
				case 5: pleaseWait_freeText_setText("Warning for water"); break;
				case 6: pleaseWait_freeText_setText("Wait for second confirm"); break;
				case 7: pleaseWait_freeText_setText("It is doing cleaner cycles (12)"); break;
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
	case 1: //priv_handleCalibProdotto
		if (this.fase == 30)	{ pleaseWait_btn1_hide(); this.fase = 40; }
		break;
		
	case 2: //priv_handleCalibMacina
		if (this.fase == 1)		{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 2; }
		else if (this.fase==21) { pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=30; }
		else if (this.fase==41) { pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=50; }
		break;		
	}
}

TaskCalibMotor.prototype.onFreeBtn2Clicked	= function(ev)
{ 
	switch (this.what)
	{
	case 1: //priv_handleCalibProdotto
		break;
		
	case 2: //priv_handleCalibMacina
		if (this.fase == 1)		{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 200; break; }
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
		me.value = pleaseWait_calibration_num_getValue();
		if (parseFloat(me.value) == 0)
		{
			pleaseWait_calibration_setText("Invalid value");
			me.fase = 20;
			break;
		}
	
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
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn2_show();		
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
		me.value = pleaseWait_calibration_num_getValue();
		if (parseFloat(me.value) == 0)
		{
			pleaseWait_calibration_setText("Invalid value");
			me.fase = 20;
			break;
		}
		
		pleaseWait_calibration_setText("Storing value...");
		me.gsec = parseInt( Math.round(me.value / (TIME_ATTIVAZIONE_dSEC*0.2)) );
		pleaseWait_calibration_num_hide();
		
		da3.setCalibFactorGSec(me.motor, me.gsec);
		var v = helper_intToFixedOnePointDecimale( da3.getCalibFactorGSec(me.motor) );
		rheaSetDivHTMLByName("pageCalibration_m" +me.motor, v +"&nbsp;gr/sec");
		
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
		da3.setImpulsi(me.motor, me.impulsi);
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
function TaskTestSelezione(selNum, iAttuatore)
{
	this.timeStarted = 0;
	this.selNum = selNum;
	this.iAttuatore = iAttuatore;
	this.cpuStatus = 0;
	this.fase = 0;
}

TaskTestSelezione.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	//console.log ("TaskTestSelezione::onTimer => sel[" +this.selNum +"] attuatore[" +this.iAttuatore +"] fase[" +this.fase +"] cpu[" +this.cpuStatus +"]");
	if (this.iAttuatore == 12)
	{
		//questo è il caso del test "macinata" che prevede che prima si rimuova il gruppo, poi si macini, poi si rimetta il gruppo
		this.priv_handleTestMacina(timeElapsedMSec);
	}
	else
	{
		//nei test attuatori "normali", lascio fare il lavoro alla CPU e quando questa esce dallo stato 21, finisco pure io
		if (this.fase == 0)
		{
			this.fase = 1;
			
			//chiedo l'attivazione del motore
			//console.log ("TaskTestSelezione::ajax::testSelection");
			rhea.ajax ("testSelection", {"s":this.selNum, "d":this.iAttuatore} ).then( function(result)
			{
				if (result != "OK")
					pageSingleSelection_test_onFinish();
			})
			.catch( function(result)
			{
				pageSingleSelection_test_onFinish();
			});			
		}
		else
		{
			//aspetto almeno un paio di secondi
			if (timeElapsedMSec < 2000)
				return;
			//monitoro lo stato di cpu per capire quando esce da 21 e terminare
			if (this.cpuStatus != 21 && this.cpuStatus != 101) //21==eVMCState_TEST_ATTUATORE_SELEZIONE
				pageSingleSelection_test_onFinish();
		}
	}
}

TaskTestSelezione.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskTestSelezione.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }
TaskTestSelezione.prototype.onFreeBtn1Clicked	= function(ev)
{ 
	if (this.iAttuatore != 12)
		return;
	
	switch (this.fase)
	{
	case 1:		pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 2; break;
	case 21: 	pleaseWait_btn1_hide(); this.fase=30; break;
	case 41: 	pleaseWait_btn1_hide(); this.fase=50; break;
	}
}
TaskTestSelezione.prototype.onFreeBtn2Clicked	= function(ev)
{ 
	if (this.iAttuatore != 12)
		return;
	
	switch (this.fase)
	{
	case 1:		pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 200; break;
	case 41:	pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 2; break;
	}
}

TaskTestSelezione.prototype.onExit				= function(bSave)					{ return bSave; }

TaskTestSelezione.prototype.priv_handleTestMacina = function (timeElapsedMSec)
{
	var me = this;
	
	switch (this.fase)
	{
	case 0:
		me.fase = 1;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("Please remove the brewer, then press CONTINUE");
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn2_show();
		break;
		
	case 1:	//attendo btn CONTINUE / ABORT
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
		
	case 10:  //ok, il gruppo è scollegato, chiedo a CPU di attivare la macina
		me.fase = 11;
		pleaseWait_calibration_setText ("Grinder is running");
		rhea.ajax ("testSelection", {"s":me.selNum, "d":me.iAttuatore} ).then( function(result)
		{
			if (result == "OK")
				me.fase = 20;
			else
				me.fase = 10;
		})
		.catch( function(result)
		{
			me.fase = 200;
		});					
	break;
		
	case 11: //attendo risposta di CPU
		break;
		
	//attendo la fine della macinata (ovvero quando la CPU passa in stato != 21)
	case 20:	me.fase = 21; break;
	case 21:	me.fase = 22; break;
	case 22:	me.fase = 23; break;
	case 23:
		if (me.cpuStatus != 21 && me.cpuStatus != 101) //21==eVMCState_TEST_ATTUATORE_SELEZIONE
			me.fase = 40;
		break;
	
	case 40: //chiedo di rimettere a posto il gruppo
		pleaseWait_calibration_setText("Place the brewer into position, then press CONTINUE or press REPEAT to grind again");
		pleaseWait_btn1_show();
		pleaseWait_btn2_setText("REPEAT");
		pleaseWait_btn2_show();		
		me.fase = 41;
		break;
		
	case 41:
		break;
		
	case 50: //verifico che il gruppo sia collegato, altrimenti goto 40
		rhea.ajax ("getGroupState", "").then( function(result)
		{
			if (result=="1")
				me.fase = 200;
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
		
	case 200:
		me.fase = 201;
		pleaseWait_btn1_hide();
		pleaseWait_calibration_hide();
		pageSingleSelection_test_onFinish();
		break;
	}
	
}


/**********************************************************
 * TaskDevices
 */
function TaskDevices()
{
	this.what = 0;
	this.fase = 0;
	this.firstTimeMacina1 = 1;
	this.firstTimeMacina2 = 1;
	this.cpuStatus = 0;
	this.enterQueryMacinePos();
}

TaskDevices.prototype.enterQueryMacinePos = function()
{	
	this.what = 0;
	this.fase = 0;
}

TaskDevices.prototype.enterSetMacinaPos = function(macina_1o2, targetValue)
{	
	this.what = 1;
	this.fase = 0;
	this.macina = macina_1o2;
	pleaseWait_show();
	rhea.sendStartPosizionamentoMacina(macina_1o2, targetValue);
}

TaskDevices.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskDevices.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }
TaskDevices.prototype.onFreeBtn1Clicked	 = function(ev)							{}
TaskDevices.prototype.onFreeBtn2Clicked	 = function(ev)							{}
TaskDevices.prototype.onExit			 = function(bSave)						{ return bSave; }


TaskDevices.prototype.onTimer = function (timeNowMsec)
{
	if (this.what == 0)
		this.priv_handleRichiestaPosizioneMacina();
	else
		this.priv_handleRegolazionePosizioneMacina();
}

TaskDevices.prototype.priv_handleRichiestaPosizioneMacina = function()
{
	var me = this;
	if (this.fase == 0)
	{
		//chiede la posizione della macina 1
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
		//chiede la posizione della macina 2
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

TaskDevices.prototype.priv_handleRegolazionePosizioneMacina = function()
{
	switch (this.fase)
	{
		case 0: this.fase=1; break;
		case 1: this.fase=2; break;
		case 2: 
			this.priv_queryMacina(this.macina);
			this.fase=3; 
			break;
		
		case 3: 
			//a questo punto CPU dovrebbe essere in stato 102 e dovrebbe rimanerci fino a fine operazione
			if (this.cpuStatus != 102 && this.cpuStatus != 101)
			{
				this.enterQueryMacinePos();
				pleaseWait_hide();
				return;
			}
			
			this.fase = 2;
			break;
	}
}

TaskDevices.prototype.priv_queryMacina = function(macina_1o2)
{
	rhea.ajax ("getPosMacina", {"m":macina_1o2}).then( function(result)
	{
		var obj = JSON.parse(result);
		rheaSetDivHTMLByName("pageDevices_vg" +macina_1o2, obj.v);
	})
	.catch( function(result)
	{
	});			
}

/**********************************************************
 * TaskDisintall
 */
function TaskDisintall()
{
	this.what = 0;
	this.fase = 0;
	this.cpuStatus = 0;
}

TaskDisintall.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskDisintall.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }
TaskDisintall.prototype.onExit			 = function(bSave)						{ return bSave; }

TaskDisintall.prototype.onFreeBtn1Clicked	 = function(ev)						
{
	switch (this.fase)
	{
		case 1:  pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=10; break;
		case 11: pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=20; break;
		case 21: pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=30; break;
		
	}
}
TaskDisintall.prototype.onFreeBtn2Clicked	 = function(ev)						
{
	pleaseWait_btn1_hide(); 
	pleaseWait_btn2_hide();
	this.fase = 99;
}


TaskDisintall.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	switch (this.fase)
	{
	case 0:
		this.fase = 1;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("DISINTALLATION<br><br>Is driptray empty?");
		pleaseWait_btn1_setText("YES - CONTINUE");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 1:
		break;
		
	case 10:
		this.fase = 11;
		pleaseWait_calibration_setText("DISINTALLATION<br><br>Please remove coffee grounds, then press CONTINUE");
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 11:
		break;
		
	case 20:
		this.fase = 21;
		pleaseWait_calibration_setText("DISINTALLATION<br><br>Press START DISINSTALLATION to continue, ABORT to cancel the operation");
		pleaseWait_btn1_setText("START DISINSTALLATION");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 21:
		break;
		
	case 30:
		this.fase = 31;
		pleaseWait_calibration_setText("DISINTALLATION is running, please wait...");
		rhea.sendStartDisintallation();
		
	case 31: this.fase = 32; break;
	case 32: this.fase = 33; break;
	case 33: this.fase = 34; break;
	case 34: 
		//a questo punto CPU dovrebbe già essere in stato eVMCState_DISINSTALLAZIONE (13)
		//quando ha finito, finisco pure io
		if (this.cpuStatus != 13 && this.status != 101)
			this.fase = 40;
		break;
		
	case 40:
		pleaseWait_calibration_setText("DISINTALLATION finished, please shut down the machine");
		this.fase = 41;
		break;
		
	case 41:
		break;
		
	case 99:
		pleaseWait_hide();
		pageMaintenance_disinstall_onAbort();
		break;
	}
}

/**********************************************************
 * TaskDataAudit
 */
function TaskDataAudit()
{
	this.what = 0;
	this.fase = 0;
	this.cpuStatus = 0;
	this.fileID = 0;
	this.buffer = null;
	this.bufferSize = 0;
}

TaskDataAudit.prototype.onEvent_cpuStatus  = function(statusID, statusStr)		{ this.cpuStatus = statusID; pleaseWait_setTextLeft (statusStr +" [" +statusID +"]"); }
TaskDataAudit.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ pleaseWait_setTextRight(msg); }
TaskDataAudit.prototype.onExit			   = function(bSave)					{ return bSave; }

TaskDataAudit.prototype.onFreeBtn1Clicked	 = function(ev)						
{
	switch (this.fase)
	{
		case 202:	pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 210; break;
	}
}

TaskDataAudit.prototype.onFreeBtn2Clicked	 = function(ev)						
{
}


TaskDataAudit.prototype.onTimer = function (timeNowMsec)
{
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	var me = this;
	//console.log ("TaskDataAudit fase[" +this.fase +"]");
	switch (this.fase)
	{
		case 0:
			rhea.onEvent_readDataAudit = function(status, kbSoFar, fileID) 
										{ 
											//console.log("status[" +status +"], kbSoFar[" +kbSoFar +"], fileID[" +fileID +"]"); 
											switch (status)
											{
											case 0://in progress
												pleaseWait_freeText_setText ("DOWNLOADING EVA-DTS: " +kbSoFar +" Kb");
												break;
												
											case 1: //finished ok
												me.fase = 10;
												me.fileID = fileID;
												break;
											
											default:	//errore
												me.status = 200;
												break;
											}
										}
			this.fase = 1;
			pleaseWait_show();
			pleaseWait_freeText_setText ("REQUESTING EVA-DTS, please wait");
			pleaseWait_freeText_show();
			rhea.sendStartDownloadDA3();			
			break;
			
		case 1: //download eva-dts in corso
			break;
			
		case 10: //eva dts scaricato
			rhea.onEvent_readDataAudit = function(status, kbSoFar, fileID) {};
			me.fase = 20;
			pleaseWait_freeText_appendText ("<br>Done, processing data, please wait<br>");
			break;
			
		case 20: //inizio il download della versione "packed" dell'eva-dts che la GPU ha generato durante la fase precedente
			me.fase = 21;
			rhea.filetransfer_startDownload ("packaudit" +me.fileID, me, TaskDataAudit_load_onStart, TaskDataAudit_load_onProgress, TaskDataAudit_load_onEnd);
			break;
			
		case 21: //attende fine download file packed
			break;
			
			
			
		case 200: //errore downloading eva-dts
			rhea.onEvent_readDataAudit = function(status, kbSoFar, fileID) {};
			pleaseWait_freeText_appendText("Error downloading EVA-DTS. Please try again later");
			me.fase = 201;
			break;
			
		case 201: //mostra btn close e ne aspetta la pressione
			pleaseWait_btn1_setText("CLOSE");
			pleaseWait_btn1_show();
			me.fase = 202;
			break;			
		case 202: //attende pressione btn1
			break;

			
		case 210: //fine
			this.fase = 211;
			pageDataAudit_downloadEVA_onFinish();
			break;
			
		default:
			break;
	}
}


function TaskDataAudit_load_onStart(userValue)					{ }
function TaskDataAudit_load_onProgress()						{ }
function TaskDataAudit_load_onEnd (theTask, reasonRefused, obj)
{
	if (reasonRefused != 0)
	{
		pleaseWait_freeText_appendText ("Error, reason[" +reasonRefused +"]");
		theTask.fase = 201;
		return;
	}

	theTask.buffer = new Uint8Array(obj.fileSize);
	theTask.bufferSize = parseInt(obj.fileSize);
	for (var i=0; i<obj.fileSize; i++)
		theTask.buffer[i] = obj.fileBuffer[i];
	theTask.fase = 210;
}


/********************************************************
 * TaskResetEVA
 */
function TaskResetEVA()																{ this.fase = 0;}
TaskResetEVA.prototype.onEvent_cpuStatus 	= function(statusID, statusStr)			{}
TaskResetEVA.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskResetEVA.prototype.onFreeBtn1Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 10; }
TaskResetEVA.prototype.onFreeBtn2Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 99; }
TaskResetEVA.prototype.onExit				= function(bSave)						{ return bSave; }
TaskResetEVA.prototype.onTimer 				= function(timeNowMsec)					
{
	console.log ("TaskResetEVA::fase[" +this.fase +"]");
	switch (this.fase)
	{
	case 10: //do reset
		this.fase = 11;
		var me = this;		
		rhea.ajax ("EVArst", "")
			.then( function(result) 
			{
				me.fase = 90;
			})
			.catch( function(result)
			{
				me.fase = 99;				
			});
		break;
	
	case 11:
		break;
		
	case 90:
		rheaSetDivHTMLByName("pageDataAudit_lastDownload", "");
		rheaSetDivHTMLByName("pageDataAudit_o", "");
		rheaSetDivHTMLByName("pageDataAudit_t", "");
		rheaSetDivHTMLByName("pageDataAudit_p", "");
		this.fase = 99;
		break;

		
	case 99: //fine
		pleaseWait_hide();
		this.fase = 0;
		
		break;
		
	default:
		break;
		
	}
}


/********************************************************
 * TaskP15
 */
function TaskP15()																{ this.nextTimeSendP15 = 0;}
TaskP15.prototype.onEvent_cpuStatus 	= function(statusID, statusStr)			{}
TaskP15.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskP15.prototype.onFreeBtn1Clicked	= function(ev)							{}
TaskP15.prototype.onFreeBtn2Clicked	= function(ev)							{}
TaskP15.prototype.onExit				= function(bSave)						{ return bSave; }
TaskP15.prototype.onTimer 				= function(timeNowMsec)					
{
	if (timeNowMsec >= this.nextTimeSendP15)
	{
		this.nextTimeSendP15 = timeNowMsec + 5000;
		var buffer = new Uint8Array(1);
		buffer[0] = 66;
		rhea.sendGPUCommand ("E", buffer, 0, 0);		
		console.log ("p15");
	}
}
