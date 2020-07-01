/********************************************************
 * Task
 *
 * Questo è il template delle classi "task".
 * Tutte le classi "derivate", devono implementare i metodi "on"
 */
function TaskVoid()																{}
TaskVoid.prototype.onTimer 				= function(timeNowMsec)					{}
TaskVoid.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskVoid.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskVoid.prototype.onFreeBtn1Clicked	= function(ev)							{}
TaskVoid.prototype.onFreeBtn2Clicked	= function(ev)							{}
TaskVoid.prototype.onFreeBtnTrickClicked= function(ev)							{}


/**********************************************************
 * TaskTemperature
 */
function TaskTemperature()																{ this.nextTimeCheckMSec=0;}
TaskTemperature.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskTemperature.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskTemperature.prototype.onFreeBtn1Clicked	= function(ev)								{}
TaskTemperature.prototype.onFreeBtn2Clicked	= function(ev)								{}
TaskTemperature.prototype.onFreeBtnTrickClicked= function(ev)							{}
TaskTemperature.prototype.onTimer 				= function(timeNowMsec)
{
	if (timeNowMsec >= this.nextTimeCheckMSec)
	{
		this.nextTimeCheckMSec = timeNowMsec + 3000;
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
 * TaskMaintenance
 */
function TaskMaintenance()																{ this.nextTimeCheckMSec=0;}
TaskMaintenance.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskMaintenance.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskMaintenance.prototype.onFreeBtn1Clicked	= function(ev)								{}
TaskMaintenance.prototype.onFreeBtn2Clicked	= function(ev)								{}
TaskMaintenance.prototype.onFreeBtnTrickClicked= function(ev)							{}
TaskMaintenance.prototype.onTimer 				= function(timeNowMsec)
{
	if (timeNowMsec >= this.nextTimeCheckMSec)
	{
		this.nextTimeCheckMSec = timeNowMsec + 3000;
		rhea.ajax ("getVandT", "").then( function(result)
		{
			var data = JSON.parse(result);
			rheaSetDivHTMLByName ("pageMaintenance_volt", helper_intToFixedOnePointDecimal(data.v) +" V");
			rheaSetDivHTMLByName ("pageMaintenance_temperature", pageMaintenance_formatHTMLForTemperature(data));
		})
		.catch( function(result)
		{
		});		
	}
}

/**********************************************************
 * TaskCleaning
 */
function TaskCleaning (whichWashIN, isEspresso)
{
	this.timeStarted = 0;
	this.isEspresso = isEspresso;
	this.cpuStatus = 0;
	this.whichWash = whichWashIN;
	this.fase = 0;
	this.btn1 = 0;
	this.btn2 = 0;
	this.btnTrick = 0;
	this.prevFase = 99999;
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

TaskCleaning.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL (statusStr); }
TaskCleaning.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }

TaskCleaning.prototype.onFreeBtn1Clicked	= function(ev)						{ rhea.sendButtonPress(this.btn1); pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); pleaseWait_btnTrick_hide();}
TaskCleaning.prototype.onFreeBtn2Clicked	= function(ev)						{ rhea.sendButtonPress(this.btn2); pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); pleaseWait_btnTrick_hide();}
TaskCleaning.prototype.onFreeBtnTrickClicked= function(ev)						{ rhea.sendButtonPress(this.btnTrick); pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); pleaseWait_btnTrick_hide();}

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
				case 0: pleaseWait_freeText_setText("Brewer Cleaning is not started (or ended)"); break;
				case 1: pleaseWait_freeText_setText("Brewer Cleaning is started"); break; //HC_STEP_STARTED
				case 2: pleaseWait_freeText_setText("Brewer placed"); break; //HC_STEP_BRW_PLACED
				case 3: pleaseWait_freeText_setText("Put pastille and push START"); break; //HC_STEP_TABLET
				case 4: pleaseWait_freeText_setText("Infusion"); break; //HC_STEP_INFUSION                     
				case 5: pleaseWait_freeText_setText("Brewer cleaning cycles 1"); break; // HC_STEP_BRW_1
				case 6: pleaseWait_freeText_setText("Brewer cleaning cycles 2"); break;	// HC_STEP_BRW_2
				case 7: pleaseWait_freeText_setText("Brewer cleaning cycles 3"); break;	// HC_STEP_BRW_3
				case 8: pleaseWait_freeText_setText("Brewer cleaning cycles 4"); break;	// HC_STEP_BRW_4
				case 9: pleaseWait_freeText_setText("Brewer cleaning cycles 5"); break;	// HC_STEP_BRW_5
				case 10: pleaseWait_freeText_setText("Brewer cleaning cycles 6"); break;	// HC_STEP_BRW_6
				
				case 11: pleaseWait_freeText_setText("Repeat cleaning ?"); break;	//HC_STEP_BRW_REPEAT
				case 12: pleaseWait_freeText_setText("Brewer placed in brush position, press CONTINUE when finished."); break;	//HC_STEP_BRW_BRUSH_POSITION
				case 13: pleaseWait_freeText_setText("Skip final coffee or make a coffee"); break; //HC_STEP_BRW_SKIP_FINAL_COFFEE
				case 14: pleaseWait_freeText_setText("Coffee delivery"); break; //HC_STEP_COFFEE_DELIVERY
				
				case 15:	//HC_STEP_MIXER_1
				case 16:	//HC_STEP_MIXER_2
				case 17:	//HC_STEP_MIXER_3
				case 18:	//HC_STEP_MIXER_4
					cleanMixNum = me.fase-14;
					if (me.isEspresso)
						cleanMixNum++;
					pleaseWait_freeText_setText("Cleaning " +cleanMixNum);
					break; 	
				default: pleaseWait_freeText_setText(""); break;
			}
			pleaseWait_freeText_show();
				
			if (me.fase != me.prevFase)
			{
				me.prevFase = me.fase;

				if (me.btn1 == 0)
					pleaseWait_btn1_hide();
				else
				{
					if (me.fase == 4)
					{
						//qui, la CPU invia il BTN 3 che serve per "debug" per saltare la fase di dissoluzione della tab.
						//Non mostro il tasto, ma uso il btn trick che è in semi trasparenza nell'angolo in alto a dx. Gli operatore
						//che conoscono il trucco, possono cliccare sul btn invisibile per skippare questa fase
						me.btnTrick = me.btn1;
						pleaseWait_btnTrick_show();	
					}
					else
					{
						var btnText = "BUTTON " +me.btn1;
						switch (me.fase)
						{
							case 3:  btnText = "START"; break; //HC_STEP_TABLET
							case 11: btnText = "NO"; break; //HC_STEP_BRW_REPEAT
							case 12: btnText = "CONTINUE"; break; //HC_STEP_BRW_BRUSH_POSITION
							case 13: btnText = "SKIP COFFEE"; break; //HC_STEP_BRW_SKIP_FINAL_COFFEE
						}
						pleaseWait_btn1_setText (btnText);
						pleaseWait_btn1_show();	
					}
				}
				
				if (me.btn2 == 0)
					pleaseWait_btn2_hide();
				else
				{
					var btnText = "BUTTON " +me.btn2;
					switch (me.fase)
					{
						case 11: btnText = "YES"; break;//HC_STEP_BRW_REPEAT
						case 13: btnText = "HAVE A COFFEE"; break; //HC_STEP_BRW_SKIP_FINAL_COFFEE
					}
					pleaseWait_btn2_setText (btnText);
					pleaseWait_btn2_show();	
				}
			} //if (me.fase != me.prevFase)
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
	//termino quando lo stato della CPU diventa != da LAVAGGIO_MILKER
	if (timeElapsedMSec > 3000 && this.cpuStatus != 23) //23==LAVAGGIO_MILKER
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
				case 7: pleaseWait_freeText_setText("Doing cleaner cycles (12)"); break;
				default: pleaseWait_freeText_setText(""); break;
			}
			pleaseWait_freeText_show();
			
			if (me.btn1 == 0)
				pleaseWait_btn1_hide();
			else
			{
				switch (me.fase)
				{
					default: pleaseWait_btn1_setText ("BUTTON " +me.btn2); break;
					case 2:  pleaseWait_btn1_setText ("START"); break;
					case 3:
					case 5:  pleaseWait_btn1_setText ("CONTINUE"); break;
					case 6:  pleaseWait_btn1_setText ("CONFIRM"); break;
				}
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
	this.cpuStatus = 0;
	this.bAlsoCalcImpulses = 1;
}

TaskCalibMotor.prototype.startCalibrazioneMacina = function (macina1o2, bAlsoCalcImpulses)
{
	var motor = 10 + macina1o2;
	this.startMotorCalib(motor);
	this.bAlsoCalcImpulses = bAlsoCalcImpulses;
}


TaskCalibMotor.prototype.startMotorCalib = function (motorIN) //motorIN==11 per macina1, 12 per macina2
{
	this.bAlsoCalcImpulses = 0;
	this.timeStarted = 0;
	this.fase = 0;
	this.motor = motorIN;
	this.impulsi = 0;
	this.value = 0;
	this.amIAskingForVGrindPos = 0;
	
	pleaseWait_calibration_varigrind_hide();
	pleaseWait_calibration_motor_hide();
	
	this.what = 0;
	if (motorIN == 11 || motorIN == 12)
		this.what = 2;
	else
		this.what = 1;
}

TaskCalibMotor.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID;}
TaskCalibMotor.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg);}
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
TaskCalibMotor.prototype.onFreeBtnTrickClicked= function(ev)							{}

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
		pleaseWait_calibration_setText("Please wait while motor is running"); //Please wait while motor is running
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
		
		//mostro la riga che chiede se si vuole rifare il movimento del motore
		pleaseWait_calibration_motor_show();
		pleaseWait_calibration_varigrind_hide();		

		
		pleaseWait_calibration_setText("When you're done, please enter the quantity in grams of the LAST ground, then press CONTINUE"); //Please enter the quantity, then press CONTINUE
		pleaseWait_calibration_num_setValue(0);
		pleaseWait_calibration_num_show();
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		break;
		
	case 30: //attende che l'utente prema uno dei pulsanti visibili a video
		break;
		
	case 40:
		pleaseWait_calibration_motor_hide();
		me.value = pleaseWait_calibration_num_getValue();
		if (parseFloat(me.value) == 0)
		{
			pleaseWait_calibration_setText("Invalid value");
			me.fase = 20;
			break;
		}
	
		pleaseWait_calibration_setText("Storing value ...");
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
		var v = helper_intToFixedOnePointDecimal( da3.getCalibFactorGSec(me.motor) );
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
	if (da3.read8(7554) == 0)
		TIME_ATTIVAZIONE_dSEC = 40;
	//console.log ("Tempo attivazione macina [" +TIME_ATTIVAZIONE_dSEC +"] dsec");
	
	var me = this;
	//console.log ("TaskCalibMotor::fase[" +me.fase +"]");
	switch (this.fase)
	{
	case 0:
		me.fase = 1;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("Please remove the brewer, then press CONTINUE"); //Please remove the brewer, then press CONTINUE
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn2_show();	
		uiStandAloneVarigringTargetPos.setValue(0);
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
		pleaseWait_calibration_setText("Please wait while motor is running"); //Please wait while motor is running
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
		
		//mostro le regolazioni per il gruppo caffè
		pleaseWait_calibration_motor_hide();
		pleaseWait_calibration_varigrind_show();		
		
		pleaseWait_calibration_setText("When you're done, please enter the quantity in grams of the LAST ground, then press CONTINUE"); //Please enter the quantity, then press CONTINUE
		pleaseWait_calibration_num_setValue(0);
		pleaseWait_calibration_num_show();
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		break;
		
	case 21: //attendo pressione di continue per terminare, oppure GRIND AGAIN o SET per calibrare l'apertura del vgrind
		
		//periodicamente, chiedo la posizione attuale della macina del VGrind
		if (da3.isGruppoVariflex())
		{
			if (!me.amIAskingForVGrindPos)
			{
				me.amIAskingForVGrindPos = 1;
				rhea.ajax ("getPosMacina", {"m":(me.motor-10)}).then( function(result)
				{
					var obj = JSON.parse(result);
					rheaSetDivHTMLByName("pagePleaseWait_calibration_1_vg", obj.v);
					if (uiStandAloneVarigringTargetPos.getValue() == 0)					
						uiStandAloneVarigringTargetPos.setValue(obj.v)
					me.amIAskingForVGrindPos = 0;
				})
				.catch( function(result)
				{
					me.amIAskingForVGrindPos = 0;
				});		
			}
		}	
		break;
		
		
	case 25:	//qui ci andiamo se siamo in fase 21 e l'utente preme il btn SET per impostare una nuova apertura del VGrind
		pleaseWait_calibration_setText("Please wait while the varigrind is adjusting its position"); //Please wait while the varigrind is adjusting its position
		rhea.sendStartPosizionamentoMacina((me.motor-10), uiStandAloneVarigringTargetPos.getValue());
		me.fase = 26;
		break;
		
	case 26:	me.fase = 27; break;
	case 27:	me.fase = 28; break;
	case 28:
		if (da3.isGruppoVariflex())
		{
			rhea.ajax ("getPosMacina", {"m":(me.motor-10)}).then( function(result)
			{
				var obj = JSON.parse(result);
				rheaSetDivHTMLByName("pagePleaseWait_calibration_1_vg", obj.v);
				pleaseWait_calibration_setText("Please wait while the varigrind is adjusting its position" +"  [" +obj.v +"]"); //Please wait while the varigrind is adjusting its position  [current pos]
			})
			.catch( function(result)
			{
			});	
		}
		me.fase = 29;
		break;	

	case 29:
		//a questo punto CPU dovrebbe essere in stato 102 e dovrebbe rimanerci fino a fine operazione
		if (me.cpuStatus != 102 && me.cpuStatus != 101)
		{
			//ho finito, torno alla schermata dove è possibile macinare di nuovo, inputare i gr e calibrare il vgrind
			me.fase = 20;
		}
		else
			me.fase = 28;		
		break;
		
	case 30:
		me.value = pleaseWait_calibration_num_getValue();
		if (parseFloat(me.value) == 0)
		{
			pleaseWait_calibration_setText("Invalid value");
			me.fase = 20;
			break;
		}
		
		pleaseWait_calibration_varigrind_hide();
		pleaseWait_calibration_setText("Storing value ...");
		me.gsec = parseInt( Math.round(me.value / (TIME_ATTIVAZIONE_dSEC*0.2)) );
		pleaseWait_calibration_num_hide();
		
		da3.setCalibFactorGSec(me.motor, me.gsec);
		//var v = helper_intToFixedOnePointDecimal( da3.getCalibFactorGSec(me.motor) );
		//rheaSetDivHTMLByName("pageCalibration_m" +me.motor, v +"&nbsp;gr/sec");
		
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
		pleaseWait_calibration_setText("Place the brewer into position, then press CONTINUE"); //Place the brewer into position, then press CONTINUE
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
		
	case 60: //gruppo è stato ricollegato, procedo con il calcolo impulsi se richiesto
		if (me.bAlsoCalcImpulses==0)
			me.fase = 200;
		else
		{
			me.fase = 65;
			pleaseWait_calibration_show();
			pleaseWait_calibration_setText("Impulse calculation in progress, please wait"); //Impulse calculation in progress, please wait
			rhea.ajax ("startImpulseCalc", { "m":me.motor, "v":me.value}).then( function(result)
			{
				//me.fase = 70;
				setTimeout ( function() { me.fase = 70; }, 3000);
			})
			.catch( function(result)
			{
				me.fase = 60;
			});
		}
		break;		
			
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
		da3.setImpulsi(me.motor, me.impulsi);

		var s = me.impulsi.toString();
		while (s.length < 3) s = "0" +s;		
		pleaseWait_calibration_setText("Impulse: " +s.substr(0,1) +"." +s.substr(1,2));
		
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
			if (this.cpuStatus != 21 && this.cpuStatus != 3 && this.cpuStatus != 101 && this.cpuStatus != 105) //21==eVMCState_TEST_ATTUATORE_SELEZIONE
				pageSingleSelection_test_onFinish();
		}
	}
}

TaskTestSelezione.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL (statusStr); }
TaskTestSelezione.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }
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

TaskTestSelezione.prototype.onFreeBtnTrickClicked= function(ev)							{}

TaskTestSelezione.prototype.priv_handleTestMacina = function (timeElapsedMSec)
{
	var me = this;
	
	switch (this.fase)
	{
	case 0:
		me.fase = 1;
		pleaseWait_show();
		pleaseWait_calibration_show();
		pleaseWait_calibration_setText("Please remove the brewer, then press CONTINUE"); //Please remove the brewer, then press CONTINUE
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
		pleaseWait_calibration_setText ("Grinder is running"); //Grinder is running
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
		pleaseWait_calibration_setText("Place the brewer into position and then press CONTINUE, or press REPEAT to grind again"); //Place the brewer into position then press CONTINUE, or press REPEAT to grind again
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
	this.firstTimeMacina1 = 2;
	this.firstTimeMacina2 = 2;
	this.cpuStatus = 0;
	this.selNum = 0;
	this.enterQueryMacinePos(1);
}

TaskDevices.prototype.enterQueryMacinePos = function(macina_1o2)
{	
	this.what = 0;
	this.fase = 0;
	this.whichMacinaToQuery = macina_1o2;
}

TaskDevices.prototype.enterSetMacinaPos = function(macina_1o2, targetValue)
{	
	this.what = 1;
	this.fase = 0;
	this.macina = macina_1o2;
	pleaseWait_show();
	rhea.sendStartPosizionamentoMacina(macina_1o2, targetValue);
}

TaskDevices.prototype.runSelection = function(selNum)
{	
	this.what = 2;
	this.fase = 0;
	this.selNum = selNum;
	pleaseWait_show();
}

TaskDevices.prototype.runModemTest = function()
{	
	this.what = 3;
	this.fase = 0;
	pleaseWait_show();
}

TaskDevices.prototype.messageBox = function (msg)
{
	this.whatBeforeMsgBox = this.what;
	this.what = 4;
	this.fase = 0;
	pleaseWait_show();
	pleaseWait_rotella_hide();
	pleaseWait_btn1_setText("OK");
	pleaseWait_btn1_show();
	pleaseWait_freeText_show();
	pleaseWait_freeText_setText(msg);
}

TaskDevices.prototype.runTestAssorbGruppo = function()
{	
	this.what = 5;
	this.fase = 0;
	pleaseWait_show();
}

TaskDevices.prototype.runTestAssorbMotoriduttore = function()
{	
	this.what = 6;
	this.fase = 0;
	pleaseWait_show();
}
TaskDevices.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL(statusStr); }
TaskDevices.prototype.onEvent_cpuMessage = function(msg, importanceLevel)			{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }
TaskDevices.prototype.onFreeBtn1Clicked	 = function(ev)
{
	switch (this.what)
	{
	case 1:
		//siamo in regolazione apertura vgrind
		if (this.fase > 0)
		{
			pleaseWait_btn1_hide();
			
			//Attivo la macina
			rhea.ajax ("runMotor", { "m":10+this.macina, "d":50, "n":1, "p":0}).then( function(result)
			{
				setTimeout ( function() {pleaseWait_btn1_show();}, 5000);
			})
			.catch( function(result)
			{
				pleaseWait_btn1_show();
			});								
		}
		break;
		
	case 4: //msgbox
		pleaseWait_hide();
		this.what = this.whatBeforeMsgBox;
		break;
		
	case 5: //test assorb gruppo
		this.fase = 90;	//ho premuto CLOSE al termine del test
		break;

	case 6: //test assorb motoriduttore
		switch (this.fase)
		{
			case 1:	//ho premuto CONTINUE nella fase di "prego rimuovere il gruppo"
				this.fase = 2;
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;
				
			case 80: //ho premuto CONTINUE al termine del test
				this.fase = 85;		
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;
				
			case 86: //ho premuto CONTINUE nella fase di "prego rimettere a posto il gruppo"
				this.fase = 87;
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;
				
		}
		break;		
	}
}
TaskDevices.prototype.onFreeBtn2Clicked	 = function(ev)
{
	switch (this.what)
	{
	case 5: //test assorb gruppo
		switch (this.fase)
		{
			case 80: //ho premuto REPEAT nella fase finale del test
				this.fase = 0;
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;		
		}
		break;
		
	case 6: //test assorb motoriduttore
		switch (this.fase)
		{
			case 1:	//ho premuto ABORT nella fase di "prego rimuovere il gruppo"
				this.fase = 90;
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;
				
			case 80: //ho premuto REPEAT nella fase finale del test
				this.fase = 10;
				pleaseWait_btn1_hide(); pleaseWait_btn2_hide();
				break;
				
		}
		break;		
	}
}

TaskDevices.prototype.onFreeBtnTrickClicked= function(ev)						{}


TaskDevices.prototype.onTimer = function (timeNowMsec)
{
	if (this.what == 0)
		this.priv_handleRichiestaPosizioneMacina();
	else if (this.what == 1)
		this.priv_handleRegolazionePosizioneMacina();
	else if (this.what == 2)
		this.priv_handleRunSelection(timeNowMsec);
	else if (this.what == 3)
		this.priv_handleModemTest(timeNowMsec);
	else if (this.what == 5)
		this.priv_handleTestAssorbGruppo(timeNowMsec);
	else if (this.what == 6)
		this.priv_handleTestAssorbMotoriduttore(timeNowMsec);
}

TaskDevices.prototype.priv_handleRunSelection = function(timeNowMsec)
{
	if (this.fase == 0)
	{
		this.fase = 1;
		this.timeStartedMSec = timeNowMsec;
		
		rhea.ajax ("testSelection", {"s":this.selNum, "d":0} ).then( function(result)
		{
			if (result != "OK")
			{
				this.fase = 2;
				pageDevices_vgrind_runSelection_onFinish();
			}
		})
		.catch( function(result)
		{
			this.fase = 2;
			pageDevices_vgrind_runSelection_onFinish();
		});			
	}
	else if (this.fase == 1)
	{
		//aspetto almeno un paio di secondi
		if ((timeNowMsec - this.timeStartedMSec) < 2000)
			return;
		//monitoro lo stato di cpu per capire quando esce da 3 (prep bevanda)
		if (this.cpuStatus != 3 && this.cpuStatus != 101 && this.cpuStatus != 105)
		{
			this.fase = 2;
			pageDevices_vgrind_runSelection_onFinish();
		}
	}
	
}

TaskDevices.prototype.priv_handleRichiestaPosizioneMacina = function()
{
	if (da3.isGruppoMicro())
		return;	
	var me = this;
	if (this.fase == 0)
	{
		//chiede la posizione della macina
		this.fase = 1;
		rhea.ajax ("getPosMacina", {"m":this.whichMacinaToQuery}).then( function(result)
		{
			var obj = JSON.parse(result);
			if (obj.m == 1) //macina1
			{
				rheaSetDivHTMLByName("pageDevices_vg1", obj.v);
				if (me.firstTimeMacina1>0)
				{
					me.firstTimeMacina1--;
					if (me.firstTimeMacina1==0)					
						ui.getWindowByID("pageDevices").getChildByID("pageDevices_vg1_target").setValue(obj.v)
				}
			}
			else
			{
				rheaSetDivHTMLByName("pageDevices_vg2", obj.v);
				if (me.firstTimeMacina2 > 0)
				{
					me.firstTimeMacina2--;
					if (me.firstTimeMacina2 == 0)
						ui.getWindowByID("pageDevices").getChildByID("pageDevices_vg2_target").setValue(obj.v)
				}
			}				
			me.fase = 0;
		})
		.catch( function(result)
		{
			me.fase = 0;
		});			
		return;
	}
	else
	{
		//aspetto una risposta alla query precedente
	}

}

TaskDevices.prototype.priv_handleRegolazionePosizioneMacina = function()
{
	switch (this.fase)
	{
		case 0: 
			this.fase=1; 
			
			pleaseWait_freeText_setText("Please wait while the varigrind is adjusting its position"); //Please wait while the varigrind is adjusting its position
			pleaseWait_freeText_show();
			/*pleaseWait_freeText_setText("While the varigrind is opening/closing, you can press RUN GRINDER to run the grinder in order to facilitate the operation.");
			pleaseWait_freeText_show();
			pleaseWait_btn1_setText("RUN GRINDER");
			pleaseWait_btn1_show();
			*/
			break;
		case 1: this.fase=2; break;
		case 2: 
			this.priv_queryMacina(this.macina);
			this.fase=3; 
			break;
		
		case 3: 
			//a questo punto CPU dovrebbe essere in stato 102 e dovrebbe rimanerci fino a fine operazione
			if (this.cpuStatus != 102 && this.cpuStatus != 101)
			{
				this.enterQueryMacinePos(this.macina);
				pleaseWait_hide();
				return;
			}
			
			this.fase = 2;
			break;
	}
}

TaskDevices.prototype.priv_queryMacina = function(macina_1o2)
{
	if (da3.isGruppoMicro())
		return;	
	rhea.ajax ("getPosMacina", {"m":macina_1o2}).then( function(result)
	{
		var obj = JSON.parse(result);
		rheaSetDivHTMLByName("pageDevices_vg" +macina_1o2, obj.v);
		pleaseWait_freeText_setText("Please wait while the varigrind is adjusting its position" +"  [" +obj.v +"]"); //Please wait while the varigrind is adjusting its position [current_value]
	})
	.catch( function(result)
	{
	});			
}


TaskDevices.prototype.priv_handleModemTest = function(timeNowMsec)
{
	var me = this;
	switch (me.fase)
	{
	case 0:
		pleaseWait_freeText_setText ("Modem test is starting...");
		pleaseWait_freeText_show();
		me.fase = 1;
		rhea.ajax ("startModemTest", "" ).then( function(result)
		{
			if (result == "OK")
				me.fase = 10;
			else
				me.fase = 90;
		})
		.catch( function(result)
		{
			me.fase = 90;
		});			
		break;


	case 1:
		//sono in attesa della risposta al comando "startModemTest"
		break;
		
	case 10:
		//ho ricevuto l'OK dal comando startModemTest. La CPU dovrebbe andare in stato 22 e rimanerci fino alla fine
		//della procedura di test
		//Aspetto un paio di secondi per dare tempo alla CPU di cambiare di stato
		pleaseWait_freeText_setText ("Modem test is running, please wait...");
		me.timeStartedMSec = timeNowMsec;
		me.fase = 11;
		break;
		
	case 11:
		//aspetto un paio di secondi
		if ((timeNowMsec - me.timeStartedMSec) >= 2000)
			me.fase = 20;
		break;
		
	case 20:
		//monitoro lo stato di cpu per capire quando questa esce da 22 (test_modem)
		if (me.cpuStatus != 22 && me.cpuStatus != 101 && me.cpuStatus != 105)
		{
			pleaseWait_freeText_setText ("Modem test finished");
			me.fase = 90;
		}
		break;
		
	case 90: //fine
		pleaseWait_hide();
		me.what = 0;
		break;
	}
	
}


TaskDevices.prototype.priv_handleTestAssorbGruppo = function(timeNowMsec)
{
	var me = this;
	switch (me.fase)
	{
	case 0:
		pleaseWait_freeText_setText ("Test is starting...");
		pleaseWait_freeText_show();
		pleaseWait_rotella_show();
		me.fase = 1;
		me.test_fase = 0;
		rhea.ajax ("startTestAssGrp", "" ).then( function(result)
		{
			if (result == "OK")
				me.fase = 10;
			else
				me.fase = 90;
		})
		.catch( function(result)
		{
			me.fase = 90;
		});			
		break;


	case 1:
		//sono in attesa della risposta al comando "start Test"
		break;
		
	case 10:
		//ho ricevuto l'OK dal comando start Test. Da ora in poi, pollo lo stato del test fino a che non finisce
		pleaseWait_freeText_setText ("Test is running, please wait...<br>Current phase: " +me.test_fase +"/5");
		me.fase = 11;
		break;
		
	case 11:
		//query stato del test
		rhea.ajax ("getStatTestAssGrp", "" ).then( function(result)
		{
			var obj = JSON.parse(result);
			me.test_fase= obj.fase;
			
			if (obj.esito != 0)
			{
				//errore
				pleaseWait_freeText_setText ("Test is complete.<br>Result: FAILED<br><br>");
				pleaseWait_btn1_setText("CLOSE");
				pleaseWait_btn1_show();	
				pleaseWait_btn2_setText("REPEAT");
				pleaseWait_btn2_show();	
				me.fase = 80;
			}
			else
			{
				if (obj.fase != 5)
					me.fase = 10;			
				else
				{
					//test terminato con successo
					var html = "<table class='dataAudit'>"
								+"<tr><td>&nbsp;</td><td align='center'><b>ASCENT</b></td><td align='center'><b>DESCENT</b></td></tr>"
								+"<tr><td>Medium absorption</td><td align='center'>" +obj.r1up +"</td><td align='center'>" +obj.r1down +"</td></tr>"
								+"<tr><td>Maximum absorption</td><td align='center'>" +obj.r2up +"</td><td align='center'>" +obj.r2down +"</td></tr>"
								+"<tr><td>Time</td><td align='center'>" +obj.r3up +"</td><td align='center'>" +obj.r3down +"</td></tr>"
								+"<tr><td>Medium absorption during cycle 1</td><td align='center'>" +obj.r4up +"</td><td align='center'>" +obj.r4down +"</td></tr>"
								+"<tr><td>Medium absorption during cycle 2</td><td align='center'>" +obj.r5up +"</td><td align='center'>" +obj.r5down +"</td></tr>"
								+"<tr><td>Medium absorption during cycle 3</td><td align='center'>" +obj.r6up +"</td><td align='center'>" +obj.r6down +"</td></tr>"
								+"</table>";
					pleaseWait_freeText_setText ("Test is complete.<br>Results:<br><br>" +html);		
					pleaseWait_btn1_setText("CLOSE");
					pleaseWait_btn1_show();	
					pleaseWait_btn2_setText("REPEAT");
					pleaseWait_btn2_show();	
					me.fase = 80;
				}
			}
		})
		.catch( function(result)
		{
			me.fase = 10;
		});			
		break;
		
	case 80: //attendo pressione di un tasto per finire
		pleaseWait_rotella_hide();
		break;
		
	case 90: //fine
		pleaseWait_hide();
		me.what = 0;
		break;
	}	
}

TaskDevices.prototype.priv_handleTestAssorbMotoriduttore = function(timeNowMsec)
{
	var me = this;
	switch (me.fase)
	{
	case 0: //prego rimuovere il gruppo
		me.fase = 1;
		pleaseWait_show();
		pleaseWait_freeText_setText ("Please remove the brewer, then press CONTINUE");
		pleaseWait_freeText_show();
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
		
		
		
	case 10: //inizio del test
		pleaseWait_freeText_setText ("Test is starting ...");
		pleaseWait_rotella_show();
		me.fase = 11;
		me.test_fase = 0;
		rhea.ajax ("startTestAssMotorid", "" ).then( function(result)
		{
			if (result == "OK")
				me.fase = 20;
			else
				me.fase = 80;
		})
		.catch( function(result)
		{
			me.fase = 80;
		});			
		break;


	case 11:
		//sono in attesa della risposta al comando "start Test"
		break;
		
	case 20:
		//ho ricevuto l'OK dal comando start Test. Da ora in poi, pollo lo stato del test fino a che non finisce
		pleaseWait_freeText_setText ("Test is running, please wait...<br>Current phase: " +me.test_fase +"/4");
		me.fase = 21;
		break;
		
	case 21:
		//query stato del test
		rhea.ajax ("getStatTestAssMotorid", "" ).then( function(result)
		{
			var obj = JSON.parse(result);
			me.test_fase= obj.fase;
			
			if (obj.esito != 0)
			{
				//errore
				pleaseWait_freeText_setText ("Test is complete.<br>Result: FAILED<br><br>");
				pleaseWait_btn1_setText("CLOSE");
				pleaseWait_btn1_show();	
				pleaseWait_btn2_setText("REPEAT");
				pleaseWait_btn2_show();	
				me.fase = 80;
			}
			else
			{
				if (obj.fase >= 4)
				{
					//test terminato con successo
					var html = "<table class='dataAudit'>"
								+"<tr><td>&nbsp;</td><td align='center'><b>ASCENT</b></td><td align='center'><b>DESCENT</b></td></tr>"
								+"<tr><td>Medium absorption</td><td align='center'>" +obj.r1up +"</td><td align='center'>" +obj.r1down +"</td></tr>"
								+"</table>";
					pleaseWait_freeText_setText ("Test is complete.<br>Results:<br><br>" +html);		
					pleaseWait_btn1_setText("CONTINUE");
					pleaseWait_btn1_show();	
					pleaseWait_btn2_setText("REPEAT");
					pleaseWait_btn2_show();	
					me.fase = 80;
				}
				else
					me.fase = 20;			

			}
		})
		.catch( function(result)
		{
			me.fase = 20;
		});			
		break;
		
	case 80: //attendo pressione di un tasto per proseguire
		pleaseWait_rotella_hide();
		break;
		
	case 85: //prego rimettere a posto il gruppo
		pleaseWait_freeText_setText("Place the brewer into position, then press CONTINUE"); //Place the brewer into position, then press CONTINUE
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn1_show();
		pleaseWait_rotella_show();
		me.fase = 86;
		break;
		
	case 86:	//attendo btn CONTINUE
		break;
		
	case 87: //verifico che il gruppo sia collegato, altrimenti goto 85
		rhea.ajax ("getGroupState", "").then( function(result)
		{
			if (result=="1")
				me.fase = 90;
			else
				me.fase = 85;
		})
		.catch( function(result)
		{
			me.fase = 85;
		});			
		me.fase = 88;
		break;
		
	case 88:	//attendo risposta CPU
		break;		
		
	
		
	case 90: //fine
		pleaseWait_hide();
		me.what = 0;
		break;
	}	
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

TaskDisintall.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL (statusStr); }
TaskDisintall.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }

TaskDisintall.prototype.onFreeBtn1Clicked	 = function(ev)						
{
	switch (this.fase)
	{
		case 1:  pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=10; break;
		case 11: pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=20; break;
		case 21: pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=30; break;
		case 33: pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase=36; rhea.sendButtonPress(10); break;
		
	}
}
TaskDisintall.prototype.onFreeBtn2Clicked	 = function(ev)						
{
	pleaseWait_btn1_hide(); 
	pleaseWait_btn2_hide();
	this.fase = 99;
}

TaskDisintall.prototype.onFreeBtnTrickClicked= function(ev)						{}

TaskDisintall.prototype.onTimer = function (timeNowMsec)
{
	var bBollitore400cc = 0;
	if (!da3.isInduzione())
	{
		if (!da3.isInstant())
		{
			if (da3.read8(7072)==0)
				bBollitore400cc=1;
		}
	}
	//console.log ("Bollitore da 400? [" +bBollitore400cc +"]");
	
	if (this.timeStarted == 0)
		this.timeStarted = timeNowMsec;
	var timeElapsedMSec = timeNowMsec - this.timeStarted;
	
	switch (this.fase)
	{
	case 0:
		this.fase = 1;
		pleaseWait_show();
		pleaseWait_freeText_show();
		pleaseWait_freeText_setText("DISINSTALLATION<br><br>Is driptray empty?"); //DISINTALLATION<br><br>Is driptray empty?
		pleaseWait_btn1_setText("YES - CONTINUE");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 1:
		break;
		
	case 10:
		this.fase = 11;
		pleaseWait_freeText_setText("DISINSTALLATION<br><br>Please remove coffee grounds, then press CONTINUE"); //DISINTALLATION<br><br>Please remove coffee grounds, then press CONTINUE
		pleaseWait_btn1_setText("CONTINUE");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 11:
		break;
		
	case 20:
		this.fase = 21;
		pleaseWait_freeText_setText("DISINSTALLATION<br><br>Press START DISINSTALLATION to continue, ABORT to cancel the operation"); //DISINTALLATION<br><br>Press START DISINSTALLATION to continue, ABORT to cancel the operation
		pleaseWait_btn1_setText("START DISINSTALLATION");
		pleaseWait_btn2_setText("ABORT");
		pleaseWait_btn1_show();
		pleaseWait_btn2_show();
		break;
		
	case 21:
		break;
		
	case 30:
		this.fase = 31;
		pleaseWait_freeText_setText("DISINSTALLATION is running, please wait ..."); //DISINTALLATION is running, please wait
		rhea.sendStartDisintallation();
		break;
		
	case 31: 
		this.fase = 32;
		break;
		
	case 32:
		if (!bBollitore400cc)
		{
			this.fase = 36; 
		}
		else
		{
			this.fase = 33;
			pleaseWait_freeText_setText("DISINSTALLATION<br><br>Open boiler tap then press CONTINUE ..."); //DISINSTALLATION<br><br>Open boiler tap then press CONTINUE
			pleaseWait_btn1_setText("CONTINUE");
			pleaseWait_btn1_show();
		}		
		break;
		
	case 33:
		break;
	
	case 36: this.fase = 37; break;
	case 37: this.fase = 38; break;
	case 38: 
		//a questo punto CPU dovrebbe già essere in stato eVMCState_DISINSTALLAZIONE (13)
		//quando ha finito, finisco pure io
		if (this.cpuStatus != 13 && this.status != 101)
			this.fase = 40;
		break;
		
	case 40:
		if (bBollitore400cc)
			pleaseWait_freeText_setText("DISINSTALLATION finished, please CLOSE the boiler tap and SHUT DOWN the machine"); //DISINTALLATION finished, please shut down the machine
		else
			pleaseWait_freeText_setText("DISINSTALLATION finished, please SHUT DOWN the machine"); //DISINTALLATION finished, please shut down the machine
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

TaskDataAudit.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL (statusStr); }
TaskDataAudit.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }

TaskDataAudit.prototype.onFreeBtn1Clicked	 = function(ev)						
{
	switch (this.fase)
	{
		case 202:	pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 210; break;
	}
}

TaskDataAudit.prototype.onFreeBtn2Clicked	 = function(ev)						{}
TaskDataAudit.prototype.onFreeBtnTrickClicked= function(ev)						{}


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
			rhea.sendStartDownloadDataAudit();			
			break;
			
		case 1: //download eva-dts in corso
			break;
			
		case 10: //eva dts scaricato
			rhea.onEvent_readDataAudit = function(status, kbSoFar, fileID) {};
			me.fase = 20;
			pleaseWait_freeText_appendText ("<br>Done. Now processing data, please wait<br>"); //Done, processing data, please wait
			break;
			
		case 20: //inizio il download della versione "packed" dell'eva-dts che la GPU ha generato durante la fase precedente
			me.fase = 21;
			rhea.filetransfer_startDownload ("packaudit" +me.fileID, me, TaskDataAudit_load_onStart, TaskDataAudit_load_onProgress, TaskDataAudit_load_onEnd);
			break;
			
		case 21: //attende fine download file packed
			break;
			
			
			
		case 200: //errore downloading eva-dts
			rhea.onEvent_readDataAudit = function(status, kbSoFar, fileID) {};
			pleaseWait_freeText_appendText("Error downloading EVA-DTS. Please try again later"); //Error downloading EVA-DTS. Please try again later
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
 * TaskDAResetTotals
 */
function TaskDAResetTotals()
{
	this.fase = 0;	
}
TaskDAResetTotals.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskDAResetTotals.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskDAResetTotals.prototype.onFreeBtn1Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); if (this.fase==11) this.fase = 20;}
TaskDAResetTotals.prototype.onFreeBtn2Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 90;}
TaskDAResetTotals.prototype.onFreeBtnTrickClicked= function(ev)							{}
TaskDAResetTotals.prototype.onTimer = function(timeNowMsec)
{
	var me = this;
	switch (this.fase)
	{
	case 0:
		this.fase = 1;
		pleaseWait_freeText_setText("WARNING: This procedure will RESET all EVADTS total counters.<br>It is highly recommended NOT to do this operation.<br><br>If you're sure you know what you're doing, click RESET ALL TOTAL COUNTERS button, otherwise click CANCEL");
		pleaseWait_freeText_show();
		
		pleaseWait_btn1_hide();
		pleaseWait_btn2_setText("CANCEL");
		pleaseWait_btn2_show();
		break;
		
	//aspetto qualche secondo prima di far vedere il bt "RESET ALL TOTAL"
	case 1: this.fase=2; break;
	case 2: this.fase=3; break;
	case 3: this.fase=4; break;
	case 4: this.fase=5; break;
	case 5: this.fase=10; break;
	
	case 10:
		this.fase = 11;
		pleaseWait_btn1_setText("RESET ALL TOTAL COUNTERS");
		pleaseWait_btn1_show();
		break;
		
	case 11: //attendo pressione di btn1 o 2
		break;
		
	case 20:
		//ho premuto btn1
		this.fase = 90;
		rhea.ajax ("EVArstTotals", "")
			.then( function(result) 
			{
				me.fase = 90;
			})
			.catch( function(result)
			{
				me.fase = 90;				
			});
		break;
	
	case 90:
		this.fase = 99;
		pageDataAudit_showSecretDaResetButtonWindow_finished();
		break;
	}
}


/********************************************************
 * TaskResetEVA
 */
function TaskResetEVA()																{ this.fase = 0;}
TaskResetEVA.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskResetEVA.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskResetEVA.prototype.onFreeBtn1Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 10; }
TaskResetEVA.prototype.onFreeBtn2Clicked	= function(ev)							{ pleaseWait_btn1_hide(); pleaseWait_btn2_hide(); this.fase = 99; }
TaskResetEVA.prototype.onFreeBtnTrickClicked= function(ev)							{}
TaskResetEVA.prototype.onTimer 				= function(timeNowMsec)					
{
	//console.log ("TaskResetEVA::fase[" +this.fase +"]");
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
TaskP15.prototype.onEvent_cpuStatus 	= function(statusID, statusStr, flag16)	{}
TaskP15.prototype.onEvent_cpuMessage 	= function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); }
TaskP15.prototype.onFreeBtn1Clicked	= function(ev)								{}
TaskP15.prototype.onFreeBtn2Clicked	= function(ev)								{}
TaskP15.prototype.onFreeBtnTrickClicked= function(ev)							{}
TaskP15.prototype.onTimer 				= function(timeNowMsec)					
{
	if (timeNowMsec >= this.nextTimeSendP15)
	{
		this.nextTimeSendP15 = timeNowMsec + 5000;
		var buffer = new Uint8Array(1);
		buffer[0] = 66;
		rhea.sendGPUCommand ("E", buffer, 0, 0);		
		//console.log ("p15");
	}
}




/**********************************************************
 * TaskEspressoCalib
 */
function TaskEspressoCalib()
{
	this.what = 0;
	this.fase = 0;
	this.cpuStatus = 0;
	this.selNum = 0;

	this.setMacina(1);
	this.enterQueryMacinePos();
}

TaskEspressoCalib.prototype.setMacina = function (macina1o2)
{
	this.macina1o2 = macina1o2;
	this.firstTimeMacina = 2;
	var w = ui.getWindowByID("pageExpCalib");
	w.getChildByID("pageExpCalib_vgBtnSet").hide();	
}

TaskEspressoCalib.prototype.enterQueryMacinePos = function()
{	
	this.what = 0;
	this.fase = 0;
}

TaskEspressoCalib.prototype.enterSetMacinaPos = function(targetValue)
{	
	this.what = 1;
	this.fase = 0;
	pleaseWait_show();
	rhea.sendStartPosizionamentoMacina(this.macina1o2, targetValue);
}

TaskEspressoCalib.prototype.runSelection = function(selNum)
{	
	this.what = 2;
	this.fase = 0;
	this.selNum = selNum;
	pleaseWait_show();
}

TaskEspressoCalib.prototype.messageBox = function (msg)
{
	this.what = 3;
	this.fase = 0;
	pleaseWait_show();
	pleaseWait_rotella_hide();
	pleaseWait_btn1_setText("OK");
	pleaseWait_btn1_show();
	pleaseWait_freeText_show();
	pleaseWait_freeText_setText(msg);
	
}


TaskEspressoCalib.prototype.onEvent_cpuStatus  = function(statusID, statusStr, flag16)	{ this.cpuStatus = statusID; pleaseWait_header_setTextL(statusStr); }
TaskEspressoCalib.prototype.onEvent_cpuMessage = function(msg, importanceLevel)		{ rheaSetDivHTMLByName("footer_C", msg); pleaseWait_header_setTextR(msg); }
TaskEspressoCalib.prototype.onFreeBtn1Clicked	 = function(ev)
{
	switch (this.what)
	{
	case 1:
		//siamo in regolazione apertura vgrind
		if (this.fase > 0)
		{
			pleaseWait_btn1_hide();
			
			//Attivo la macina
			rhea.ajax ("runMotor", { "m":10+this.macina, "d":50, "n":1, "p":0}).then( function(result)
			{
				setTimeout ( function() {pleaseWait_btn1_show();}, 5000);
			})
			.catch( function(result)
			{
				pleaseWait_btn1_show();
			});								
		}
		break;
		
	case 3: //message box
		pleaseWait_hide();
		this.what = 0;
		break;
	}
}
TaskEspressoCalib.prototype.onFreeBtn2Clicked	 = function(ev)						{}
TaskEspressoCalib.prototype.onFreeBtnTrickClicked= function(ev)						{}


TaskEspressoCalib.prototype.onTimer = function (timeNowMsec)
{
	if (this.what == 0)
		this.priv_handleRichiestaPosizioneMacina();
	else if (this.what == 1)
		this.priv_handleRegolazionePosizioneMacina();
	else if (this.what == 2)
		this.priv_handleRunSelection(timeNowMsec);
}

TaskEspressoCalib.prototype.priv_handleRunSelection = function(timeNowMsec)
{
	if (this.fase == 0)
	{
		this.fase = 1;
		this.timeStartedMSec = timeNowMsec;
		
		rhea.ajax ("testSelection", {"s":this.selNum, "d":0} ).then( function(result)
		{
			if (result != "OK")
			{
				this.fase = 2;
				pageExpCalib_runSelection_onFinish();
			}
		})
		.catch( function(result)
		{
			this.fase = 2;
			pageExpCalib_runSelection_onFinish();
		});			
	}
	else if (this.fase == 1)
	{
		//aspetto almeno un paio di secondi
		if ((timeNowMsec - this.timeStartedMSec) < 2000)
			return;
		//monitoro lo stato di cpu per capire quando esce da 3 (prep bevanda)
		if (this.cpuStatus != 3 && this.cpuStatus != 101 && this.cpuStatus != 105)
		{
			this.fase = 2;
			pageExpCalib_runSelection_onFinish();
		}
	}
	
}

TaskEspressoCalib.prototype.priv_handleRichiestaPosizioneMacina = function()
{
	if (da3.isGruppoMicro())
		return;
	var me = this;
	if (this.fase == 0)
	{
		//chiede la posizione della macina
		this.fase = 1;
		if (da3.isGruppoVariflex())
		{
			rhea.ajax ("getPosMacina", {"m":this.macina1o2}).then( function(result)
			{
				var obj = JSON.parse(result);
				rheaSetDivHTMLByName("pageExpCalib_vgCurPos", obj.v);
				if (me.firstTimeMacina>0)
				{
					me.firstTimeMacina--;
					if (me.firstTimeMacina==0)		
					{
						var w = ui.getWindowByID("pageExpCalib");
						w.getChildByID("pageExpCalib_vg_target").setValue(obj.v)
						w.getChildByID("pageExpCalib_vgBtnSet").show();
					}
				}
				me.fase = 0;
			})
			.catch( function(result)
			{
				me.fase = 0;
			});			
		}
		return;
	}
	else
	{
		//aspetto una risposta alla query precedente
	}

}

TaskEspressoCalib.prototype.priv_handleRegolazionePosizioneMacina = function()
{
	switch (this.fase)
	{
		case 0: 
			this.fase=1; 
			
			pleaseWait_freeText_setText("Please wait while the varigrind is adjusting its position"); //Please wait while the varigrind is adjusting its position
			pleaseWait_freeText_show();
			break;
		case 1: this.fase=2; break;
		case 2: 
			this.priv_queryMacina();
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

TaskEspressoCalib.prototype.priv_queryMacina = function()
{
	if (!da3.isGruppoVariflex())
		return;
	rhea.ajax ("getPosMacina", {"m":this.macina1o2}).then( function(result)
	{
		var obj = JSON.parse(result);
		rheaSetDivHTMLByName("pageExpCalib_vgCurPos", obj.v);
		pleaseWait_freeText_setText("Please wait while the varigrind is adjusting its position" +"  [" +obj.v +"]"); //Please wait while the varigrind is adjusting its position [current_value]
	})
	.catch( function(result)
	{
	});			
}
