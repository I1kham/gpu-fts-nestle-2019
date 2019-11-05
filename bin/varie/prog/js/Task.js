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
			console.log ("SAN WASH response: fase[" +obj.fase +"] b1[" +obj.btn1 +"] b2[" +obj.btn2 +"]");
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
			console.log ("SANWASH: error[" +result +"]");
			pleaseWait_btn1_hide();
			pleaseWait_btn2_hide();
		});	
	
}