#include "CPUBridgeServer.h"
#include "CPUBridge.h"
#include "CPUChannelCom.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaMemory.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"

using namespace cpubridge;


//***************************************************
Server::Server()
{
	localAllocator = NULL;
	chToCPU = NULL;
	logger = &nullLogger;
	langErrorCode = 0;

	runningSel.selNum = 0;
	runningSel.sub = NULL;
	runningSel.stopSelectionWasRequested = 0;
	runningSel.status = eRunningSelStatus_finished_OK;
}

//***************************************************
void Server::useLogger(rhea::ISimpleLogger *loggerIN)
{
	if (NULL == loggerIN)
		logger = &nullLogger;
	else
		logger = loggerIN;
}

//***************************************************
bool Server::start (CPUChannel *chToCPU_IN, const HThreadMsgR hServiceChR_IN)
{
	chToCPU = chToCPU_IN;
	hServiceChR = hServiceChR_IN;
	logger->log ("CPUBridgeServer::open\n");
	logger->incIndent();
	
	//recupero l'OSEvent della msgQ e lo aggiunto alla mia wait-list	
	OSEvent	hMsgQEvent;
	rhea::thread::getMsgQEvent (hServiceChR, &hMsgQEvent);
	waitList.addEvent(hMsgQEvent, u32MAX);

	memset (&cpuParamIniziali, 0, sizeof(sCPUParamIniziali));
	memset (&cpuStatus, 0, sizeof(sCPUStatus));

	localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("cpuBrigeServer");
	subscriberList.setup(localAllocator, 8);

	//apro il canale di comunicazione con la CPU fisica
	bool ret = chToCPU->isOpen();
	if (ret)	
		logger->log("Started OK\n");
	else
		logger->log("Started FAIL\n");
	logger->decIndent();

	return ret;
}

//***************************************************
void Server::close()
{
	logger->log("CPUBridgeServer::close\n");

	if (NULL == localAllocator)
		return;

	//rimuove l'OSEvent della msgQ dalla mia wait-list	
	OSEvent	hMsgQEvent;
	rhea::thread::getMsgQEvent(hServiceChR, &hMsgQEvent);
	waitList.removeEvent(hMsgQEvent);

	//notifica i subscriber
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
		notify_CPUBRIDGE_DYING(subscriberList(i)->q);
	OS_sleepMSec(500);
	
	//rimuove i subscriber
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
		priv_deleteSubscriber(subscriberList.getElem(i), false);
	subscriberList.unsetup();

	RHEADELETE (rhea::memory_getDefaultAllocator(), localAllocator);
}

//***************************************************
void Server::run()
{
	bQuit = false;
	priv_enterState_comError();

	while (bQuit == false)
	{
		switch (stato)
		{
		default:
		case eStato_comError:
			priv_handleState_comError();
			break;

		case eStato_normal:
			priv_handleState_normal();
			break;

		case eStato_selection:
			priv_handleState_selection();
			break;
		}
	}
}

//***************************************************
void Server::priv_deleteSubscriber(sSubscription *sub, bool bAlsoRemoveFromSubsriberList)
{
	waitList.removeEvent(sub->hEvent);
	rhea::thread::deleteMsgQ(sub->q.hFromCpuToOtherR, sub->q.hFromCpuToOtherW);
	rhea::thread::deleteMsgQ(sub->q.hFromOtherToCpuR, sub->q.hFromOtherToCpuW);
	if (bAlsoRemoveFromSubsriberList)
		subscriberList.findAndRemove(sub);
	localAllocator->dealloc(sub);
}

/***************************************************
 * gestisce tutti i msg in ingresso provenienti dal canale di "servizio" o dai subsriber
 */
void Server::priv_handleMsgQueues(u64 timeNowMSec, u32 timeOutMSec)
{
	//vediamo se ho dei messaggi in coda
	u8 nEvent = waitList.wait (timeOutMSec);

	for (u8 i = 0; i < nEvent; i++)
	{
		switch (waitList.getEventOrigin(i))
		{
		default:
			DBGBREAK;
			break;

		case OSWaitableGrp::evt_origin_osevent:
			if (waitList.getEventUserParamAsU32(i) == 0x0001)
			{
				//evento generato dalla msgQ di uno dei miei subscriber
				for (u32 i2 = 0; i2 < subscriberList.getNElem(); i2++)
				{
					if (OSEvent_compare(subscriberList(i2)->hEvent, waitList.getEventSrcAsOSEvent(i)))
					{
						priv_handleMsgFromSingleSubscriber(subscriberList.getElem(i2));
						break;
					}
				}
			}
			else if (waitList.getEventUserParamAsU32(i) == u32MAX)
			{
				//evento generato dalla msg Q di servizio
				priv_handleMsgFromServiceMsgQ();
			}
			break;
		}
	}
}

//***************************************************
void Server::priv_handleMsgFromServiceMsgQ()
{
	rhea::thread::sMsg msg;
	while (rhea::thread::popMsg (hServiceChR, &msg))
	{
		switch (msg.what)
		{
		case CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST:
			//qualcuno vuole iscriversi ai miei eventi
			{
				logger->log("CPUBridgeServer => new SUBSCRIPTION_REQUEST\n");
				logger->incIndent();

				//creo le msgQ
				sSubscription *sub = RHEAALLOCSTRUCT(localAllocator, sSubscription);
				rhea::thread::createMsgQ (&sub->q.hFromCpuToOtherR, &sub->q.hFromCpuToOtherW);
				rhea::thread::createMsgQ (&sub->q.hFromOtherToCpuR, &sub->q.hFromOtherToCpuW);
				subscriberList.append (sub);

				//aggiungo la msqQ alla mia waitList
				rhea::thread::getMsgQEvent(sub->q.hFromOtherToCpuR, &sub->hEvent);
				waitList.addEvent(sub->hEvent, 0x0001);

				//rispondo al thread richiedente
				HThreadMsgW hToThreadW;
				hToThreadW.initFromU32 (msg.paramU32);
				rhea::thread::pushMsg (hToThreadW, CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER, &sub->q, sizeof(sSubscriber));

				logger->log("OK");
				logger->decIndent();
			}
			break;

		default:
			logger->log("CPUBridgeServer::priv_handleMsgFromServiceMsgQ() => unknown request\n");
			break;
		}
		rhea::thread::deleteMsg(msg);
	}
}

//***************************************************
void Server::priv_handleMsgFromSingleSubscriber (sSubscription *sub)
{
	rhea::thread::sMsg msg;
	while (rhea::thread::popMsg (sub->q.hFromOtherToCpuR, &msg))
	{
		const u16 handlerID = (u16)msg.paramU32;

		switch (msg.what)
		{
		default:
			logger->log("CPUBridgeServer::priv_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
			break;

		case CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST:
			priv_deleteSubscriber(sub, true);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION:
			{
				u8 selNumber = 0x00;
				translate_CPU_START_SELECTION (msg, &selNumber);
				priv_enterState_selection (selNumber, sub);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION:
			if (stato == eStato_selection)
				runningSel.stopSelectionWasRequested = 1;
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS:
			notify_CPU_RUNNING_SEL_STATUS(sub->q, handlerID, logger, runningSel.status);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE:
			notify_CPU_FULLSTATE (sub->q, handlerID, logger, &cpuStatus);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM:
			notify_CPU_INI_PARAM (sub->q, handlerID, logger, &cpuParamIniziali);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL:
			notify_CPU_SEL_AVAIL_CHANGED(sub->q, handlerID, logger, &cpuStatus.selAvailability);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES:
			notify_CPU_SEL_PRICES_CHANGED(sub->q, handlerID, logger, cpuParamIniziali.prices, sizeof(cpuParamIniziali.prices));
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE:
			notify_CPU_NEW_LCD_MESSAGE (sub->q, handlerID, logger, &cpuStatus.LCDMsg);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT:
			notify_CPU_CREDIT_CHANGED (sub->q, handlerID, logger, cpuStatus.userCurrentCredit, sizeof(cpuStatus.userCurrentCredit));
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE:
			notify_CPU_STATE_CHANGED (sub->q, handlerID, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType);
			break;
		}
		rhea::thread::deleteMsg(msg);
	}
}

//***************************************************
void Server::priv_enterState_comError()
{
	logger->log("CPUBridgeServer::priv_enterState_comError()\n");
	stato = eStato_comError;
}

/***************************************************
 * priv_handleState_comError()
 *
 *	continua a mandare il comando "initialParam" fino a che la CPU non risponde.
 *	Quando la CPU risponde, this passa in stato eStato_normal e ritorna
 */
void Server::priv_handleState_comError()
{
	//preparo il msg da mandare alla CPU
	u8 bufferW[32];
	const u8 nBytesToSend = cpubridge::buildMsg_initialParam_C(2, 0, 0, bufferW, sizeof(bufferW));

	//provo a mandarlo ad oltranza
	while (stato == eStato_comError)
	{
		const u64 timeNowMSec = OS_getTimeNowMSec();

		//invio comando initalParam
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger))
		{
			//la CPU ha risposto, elaboro la risposta e passo in stato "normal"
			priv_parseAnswer_initialParam (answerBuffer, sizeOfAnswerBuffer);
			priv_enterState_normal();
			return;
		}

		priv_handleMsgQueues (timeNowMSec, 3000);
	}
}

//***************************************************
void Server::priv_parseAnswer_initialParam (const u8 *answer, u16 answerLen)
{
	/*	GIX 2018-12-17
		byte		significato
		0			#
		1			C
		2			lunghezza

		3			anno
		4			mese
		5			giorno

		6			hh
		7			mm
		8			ss

		9			versione sw	8 caratteri del tipo "1.4 WIP"
		10			versione sw
		11			versione sw
		12			versione sw
		13			versione sw
		14			versione sw
		15			versione sw
		16			versione sw

		17			98 btyes composti da 49 prezzi ciascuno da 2 bytes
		....

		Da qui in poi sono dati nuovi, introdotti a dicembre 2018

		115			versione protocollo. Inizialmente = 1, potrebbe cambiare in futuro

		116			ck
	*/

#ifdef PLATFORM_YOCTO_EMBEDDED
		u16 date_year = answer[3] + 2000;
		u16 date_month = answer[4];
		u16 date_dayOfMonth = answer[5];
		u16 date_hour = answer[6];
		u16 date_min = answer[7];
		u16 date_sec = answer[8];

		if (date_year * date_month * date_dayOfMonth == 0)
		{
			date_year = 2015; date_month = 3; date_dayOfMonth = 15;
			date_hour = 15; date_min = 1; date_sec = 0;
		}

		char s[256];
		sprintf(s, "date -u %02d%02d%02d%02d%04d.%02d", date_month, date_dayOfMonth, date_hour, date_min, date_year, date_sec);
		system(s);
		system("hwclock -w");
#endif

	//CPU version (string)
	memset (cpuParamIniziali.CPU_version, 0, sizeof (cpuParamIniziali.CPU_version));
	memcpy (cpuParamIniziali.CPU_version, &answer[9], 8);
		
	//prezzi
	if (answerLen > 24)
	{
		u8 k = 17;
		for (u8 i = 0; i < NUM_MAX_SELECTIONS; i++)
		{
			cpuParamIniziali.prices[i] = (u16)answer[k] + 256 * (u16)answer[k + 1];
			k += 2;
		}
	}

	//protocol version
	cpuParamIniziali.protocol_version = 0;
	if (answerLen >= 117)
		cpuParamIniziali.protocol_version = answer[115];
}


//***************************************************
void Server::priv_enterState_normal()
{
	logger->log("CPUBridgeServer::priv_enterState_normal()\n");
	stato = eStato_normal;;
}

/***************************************************
 *	priv_handleState_normal
 *
 *	Manda periodicamente il msg B (checkStatus) di richiesta di stato alla CPU.
 *	In caso di mancata risposta, passa allo stato com_error.
 *
 *	Se qualche subscriber invia il comando di startSelezione (e la selezione in questione è disponibile), allora si prova a passare in stato "selection"
 */
void Server::priv_handleState_normal()
{
	const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 250;

	u64	nextTimeSendCheckStatusMsgWasMSec = 0;
	while (stato == eStato_normal)
	{
		const u64 timeNowMSec = OS_getTimeNowMSec();

		//ogni tot, invio un msg di stato alla CPU
		if (timeNowMSec >= nextTimeSendCheckStatusMsgWasMSec)
		{
			u8 bufferW[32];
			u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B (0, langErrorCode, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger))
			{
				//la CPU non ha risposto al mio comando di stato, passo in com_error
				priv_enterState_comError();
				return;
			}

			//parso la risposta
			priv_parseAnswer_checkStatus (answerBuffer, sizeOfAnswerBuffer);

			//schedulo il prossimo msg di stato
			nextTimeSendCheckStatusMsgWasMSec = OS_getTimeNowMSec() + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;
		}

		//ci sono messaggi in ingresso?
		priv_handleMsgQueues (timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
	}
}

/***************************************************
 *	priv_parseAnswer_checkStatus
 *
 *	parsa la risposta della CPU alla mia richiesta di stato.
 *	Triggera alcuni eventi in modo da notificare chiunque sia in ascolta sulla coda di questo thread del fatto
 *	che qualcosa è cambiato.
 *	In particolare:
 *		CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED		se VMCstate/VMCerrorCode/VMCerrorType sono cambiati
 *		CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE	se è arrivato un msg LCD diverso dal precedente
 *		CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED		se il credito disponibile è cambiato
 *		CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED	se la disponibilità delle selezioni è cambiata
 */
void Server::priv_parseAnswer_checkStatus (const u8 *answer, u16 answerLen)
{
	u8 isMultilangage = 0;
	const u8 prevMsgLcdCPUImportanceLevel = cpuStatus.LCDMsg.importanceLevel;
	cpuStatus.LCDMsg.importanceLevel = 0xff;

	if (cpuParamIniziali.protocol_version >= 1)
	{
		u8 z = 81;
		cpuStatus.beepSelezioneLenMSec = answer[z + 1];
		cpuStatus.beepSelezioneLenMSec *= 256;
		cpuStatus.beepSelezioneLenMSec += answer[z];
		cpuStatus.beepSelezioneLenMSec *= 100; //trasforma in msec
		z += 2;


		if (cpuParamIniziali.protocol_version >= 2)
		{
			//un byte per indicare se la CPU sta usando il suo linguaggio oppure sta usando il linguaggio "ml" (ovvero manda i messaggi con @)
			switch (answer[z++])
			{
				default:
				case '0':
					//lang_clearErrorCode(language);
					z += 2;
					break;

				case '1':
				{
					//cpu usa un linguaggio custom, vediamo che lingua vuole
					isMultilangage = 1;
					char language_requested[4];
					language_requested[0] = answer[z++];
					language_requested[1] = answer[z++];

					//se necessario, cambio lingua
					/*const char *curLang = lang_getCurLanguage (language);
					if (curLang[0] != language_requested[0] || curLang[1] != language_requested[1])
					{
						language_requested[2] = 0x00;
						lang_open (language, language_requested);
					}*/
				}
				break;
			}


			if (cpuParamIniziali.protocol_version >= 3)
			{
				//1 byte per indicare importanza del msg di CPU (0=poco importante, 1=importante)
				//8 byte stringa con l'attuale credito
				cpuStatus.LCDMsg.importanceLevel = answer[z++];

				//se il credito è cambiato, lo memorizzo e notifico i subscriber
				if (memcmp(cpuStatus.userCurrentCredit, &answer[z], 8) != 0)
				{
					memcpy(cpuStatus.userCurrentCredit, &answer[z], 8);
					cpuStatus.userCurrentCredit[8] = 0;

					for (u32 i = 0; i < subscriberList.getNElem(); i++)
						notify_CPU_CREDIT_CHANGED (subscriberList(i)->q, 0, logger, cpuStatus.userCurrentCredit, 8);
				}
				z += 8;

				
			}

		} // if (cpuParamIniziali.protocol_version >= 2)
	} //if (cpuParamIniziali.protocol_version >= 1)


	
	//stato della CPU
	u8 bTriggerEvent = 0;
	if (cpuStatus.VMCstate != (eVMCState)answer[3])		{ cpuStatus.VMCstate = (eVMCState)answer[3]; bTriggerEvent = 1; }
	if (cpuStatus.VMCerrorCode != answer[4])			{ cpuStatus.VMCerrorCode = answer[4]; bTriggerEvent = 1; }
	if (cpuStatus.VMCerrorType != answer[5])			{ cpuStatus.VMCerrorType = answer[5]; bTriggerEvent = 1; }
	if (bTriggerEvent)
	{
		for (u32 i = 0; i < subscriberList.getNElem(); i++)
			notify_CPU_STATE_CHANGED (subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType);
	}


	cpuStatus.CupAbsentStatus_flag = answer[9] & 0x08;
	cpuStatus.bShowDialogStopSelezione = answer[9] & 0x10;

	/*  GIX 2018 05 04
		Abbiamo deciso che la GPU deve avere un modo per sapere se la CPU sta preparando
		una bevanda e, se si, se è in attesa che i sistemi di pagamento facciano il loro mestiere.
		Usiamo i bit 0x20 e 0x40 in questo modo:
			se == 0  => la CPU è una versione "vecchia" che non supporta questa nuova procedura
						per cui anche la GPU deve funzionare come faceva prima

			se == 0x01  =>  la CPU è nuova, 0x01 è il suo stato di default e vuol dire "non sto facendo nulla"
			se == 0x02  =>  la CPU ha capito che deve preparare una bevanda, è in attesa che i sistemi di pagamento rispondano
			se == 0x03  =>  la CPU ha dato l'OK alla preparazione (equivalente di BEVANDA IN PREP)
	*/
	cpuStatus.statoPreparazioneBevanda = (eStatoPreparazioneBevanda)((answer[9] & 0x60) >> 5);

	u8 selection_CPU_current = answer[10];


	//64 bytes unicode di messaggio "testuale"
	u16	msgLCD[CPU_MSG_LCD_MAX_LEN_IN_BYTES / 2];
	u8 msgLCDct = 0;
	u8 z = 11;
	u16 firstGoodChar = ' ';
	for (u8 i = 0; i < 32; i++)
	{
		msgLCD[msgLCDct] = (u16)answer[z] + (u16)answer[z + 1] * 256;
		z += 2;

		if (msgLCD[msgLCDct] != ' ' && firstGoodChar == ' ')
			firstGoodChar = msgLCD[msgLCDct];
		msgLCDct++;

		if (msgLCDct == 16)
		{
			if (!isMultilangage || firstGoodChar != '@')
				msgLCD[msgLCDct++] = ' ';
		}
	}
	msgLCD[msgLCDct] = 0;

	
	//1 bit per ogni selezione per indicare se la selezione è disponibile o no
	//Considerando che NumMaxSelections=48, dovrebbero servire 6 byte
	//ATTENZIONE che bit==0 significa che la selezione è OK, bit==1 significa KO
	//Io invece traduco al contrario, per cui per me cupStatus.selAvailability == 1 se la selezione è disponibile
	//if (cpuStatus.VMCstate != VMCSTATE_INITIAL_CHECK && cpuStatus.VMCstate != VMCSTATE_ERROR)
	{
		u8 mask = 0x01;
		u8 anythingChanged = 0;
		for (u8 i = 0; i < NUM_MAX_SELECTIONS; i++)
		{
			u8 isSelectionAvail = 1;
			if ((answer[z] & mask) != 0)
				isSelectionAvail = 0;

			if (isSelectionAvail)
			{
				if (!cpuStatus.selAvailability.isAvail(i+1))
				{
					anythingChanged = 1;
					cpuStatus.selAvailability.setAsAvail(i + 1);
				}
			}
			else
			{
				if (cpuStatus.selAvailability.isAvail(i+1))
				{
					anythingChanged = 1;
					cpuStatus.selAvailability.setAsNotAvail(i + 1);
				}
			}

			if (mask == 0x80)
			{
				mask = 0x01;
				z++;
			}
			else
				mask <<= 1;
		}

		if (anythingChanged)
		{
			for (u32 i = 0; i < subscriberList.getNElem(); i++)
				notify_CPU_SEL_AVAIL_CHANGED (subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
		}
	}

	//se non c'è nemmeno una selezione disponibile, mostro sempre e cmq il msg di CPU anche se non fosse "importante"
	if (cpuStatus.selAvailability.areAllNotAvail())
		cpuStatus.LCDMsg.importanceLevel = 0xff;

	
	//se il messaggio LCD è cambiato dal giro precedente, oppure lo stato di importanza è cambiato...
	msgLCDct *= 2;
	if (prevMsgLcdCPUImportanceLevel != cpuStatus.LCDMsg.importanceLevel || msgLCDct != cpuStatus.LCDMsg.ct || memcmp(msgLCD, cpuStatus.LCDMsg.buffer, msgLCDct) != 0)
	{
		memcpy (cpuStatus.LCDMsg.buffer, msgLCD, msgLCDct);
		cpuStatus.LCDMsg.ct = msgLCDct;
		for (u32 i = 0; i < subscriberList.getNElem(); i++)
			notify_CPU_NEW_LCD_MESSAGE (subscriberList(i)->q, 0, logger, &cpuStatus.LCDMsg);
	}

}

/***************************************************
 * priv_enterState_selection
 *
 *	ritorna true se ci sono le condizioni per iniziare una selezione. In questo caso, lo stato passa a stato = eStato_selection.
 *	In caso contrario, ritorna false e non cambia l'attuale stato.
 */
bool Server::priv_enterState_selection (u8 selNumber, const sSubscription *sub)
{
	logger->log("CPUBridgeServer::priv_enterState_selectionRequest() => [%d]\n", selNumber);

	if (stato != eStato_normal)
	{
		logger->log("  invalid request, CPUServer != eStato_normal, aborting.");
		if (sub)
			notify_CPU_RUNNING_SEL_STATUS (sub->q, 0, logger, eRunningSelStatus_finished_KO);
		return false;
	}

	if (cpuStatus.VMCstate != eVMCState_DISPONIBILE)
	{
		logger->log("  invalid request, VMCState != eVMCState_DISPONIBILE, aborting.");
		if (sub)
			notify_CPU_RUNNING_SEL_STATUS(sub->q, 0, logger, eRunningSelStatus_finished_KO);
		return false;
	}

	if (selNumber < 1 || selNumber > NUM_MAX_SELECTIONS)
	{
		logger->log("  invalid selection number, aborting.");
		if (sub)
			notify_CPU_RUNNING_SEL_STATUS(sub->q, 0, logger, eRunningSelStatus_finished_KO);
		return false;
	}


	stato = eStato_selection;
	runningSel.selNum = selNumber;
	runningSel.stopSelectionWasRequested = 0;
	runningSel.sub = sub;
	runningSel.status = eRunningSelStatus_wait;
	if (sub)
		notify_CPU_RUNNING_SEL_STATUS(sub->q, 0, logger, runningSel.status);
	return true;
}

/***************************************************
 *	priv_handleState_selection()
 *
 *	Durante questo stato ogni tot viene inviata la notifica CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS per
 *	informare il subscriber sullo stato della preparazione della bevanda.
 *
 *	Periodicamente invia il messaggio B (checkStatus) alla CPU per rimanere informati sullo stato della erogazione.
 *	In caso di mancata risposta, si passa in com_error.
 *	Quando l'erogazione termina, si torna in stato "normal".
 */
void Server::priv_handleState_selection()
{
	//mando la richiesta di stato alla CPU ogni tot msec
	const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 100;
	
	//la cpu deve passare da DISPONIBILE a "BEVANDA IN PREPARAZIONE" entro il tempo definito qui sotto
	const u32 TIMEOUT_SELEZIONE_1_MSEC = 12000;

	//una volta che la CPU è entrata in "PREPARAZIONE", deve tornare disponibile entro il tempo definito qui sotto
	const u32 TIMEOUT_SELEZIONE_2_MSEC = 240000;


	assert (runningSel.selNum >= 1 && runningSel.selNum <= NUM_MAX_SELECTIONS);


	u64	timeStartedMSec = OS_getTimeNowMSec();
	u8 bBevandaInPreparazione = 0;

	//loop fino alla fine della selezione
	u64	nextTimeSendCheckStatusMsgMSec = 0;
	while (stato == eStato_selection)
	{
		u64 timeNowMSec = OS_getTimeNowMSec();

		//ogni tot, invio un msg di stato alla CPU, altrimenti non faccio niente, rimango in attesa della fine della selezione
		//e controllo eventuali msg in ingresso dai subscriber
		if (timeNowMSec < nextTimeSendCheckStatusMsgMSec)
		{
			priv_handleMsgQueues (timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
			continue;
		}

		u8 selNumberToSend = 0;

		//al primo giro, nella richiesta di stato includo il numero di selezione da erogare
		if (nextTimeSendCheckStatusMsgMSec == 0)
			selNumberToSend = runningSel.selNum;
		else
		{
			//successivamente invio sempre 0 a meno che non sia stato premuto il btn stop selezione
			if (runningSel.stopSelectionWasRequested)
			{
				selNumberToSend = runningSel.selNum;
				runningSel.stopSelectionWasRequested = 0;
			}
		}

		//invio il msg di stato alla CPU
		u8 bufferW[32];
		u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B (selNumberToSend, langErrorCode, bufferW, sizeof(bufferW));
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger))
		{
			//la CPU non ha risposto al mio comando di stato, passo in com_error
			runningSel.status = eRunningSelStatus_finished_KO;
			if (runningSel.sub)
				notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
			priv_enterState_comError();
			return;
		}

		//parso la risposta
		priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);

		//schedulo il prossimo msg di stato
		timeNowMSec = OS_getTimeNowMSec();
		nextTimeSendCheckStatusMsgMSec = timeNowMSec + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;



		/* Teoricamente l'intero processo è ora pilotato da "statoPreparazioneBevanda"
		 * Voglio però essere sicuro di aver letto almeno una volta uno stato != da eStatoPreparazioneBevanda_doing_nothing
		 * prima di imbarcarmi nel processo.
		 * Appena vedo uno stato != da eStatoPreparazioneBevanda_doing_nothing setto bBevandaInPreparazione e parto.
		 * Se non vedo questa condizione entro 4/5 second, vuol dire che c'è qualcosa che non va e abortisco*/
		if (bBevandaInPreparazione == 0)
		{
			if (cpuStatus.statoPreparazioneBevanda != eStatoPreparazioneBevanda_doing_nothing)
				bBevandaInPreparazione = 1;
			else if (timeNowMSec - timeStartedMSec >= TIMEOUT_SELEZIONE_1_MSEC)
			{
				logger->log("priv_handleState_selection() => aborting, TIMEOUT_SELEZIONE_1_MSEC\n");
				priv_onSelezioneTerminataKO();
				return;
			}
		}
		else
		{
			switch (cpuStatus.statoPreparazioneBevanda)
			{
			default:
				logger->log("priv_handleState_selection() => statoPreparazioneBevanda invalid [%d]\n", cpuStatus.statoPreparazioneBevanda);
				priv_onSelezioneTerminataKO();
				return;

			case eStatoPreparazioneBevanda_wait:
				//sto aspettando che la CPU decida il da farsi
				if (cpuStatus.VMCstate != eVMCState_DISPONIBILE && cpuStatus.VMCstate != eVMCState_PREPARAZIONE_BEVANDA)
				{
					logger->log("priv_handleState_selection() => aborting, ero in WAIT ma CPU è andata in uno stato != da DISP o PREP");
					priv_onSelezioneTerminataKO();
					return;
				}
				else
				{
					runningSel.status = eRunningSelStatus_wait;
					if (runningSel.sub)
						//informo il subscriber che ha richiesto la bevanda che la CPU è ancora in wait
						notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
				}
				break;

			case eStatoPreparazioneBevanda_running:
				//la cpu ha dato l'OK, sta preparando la bevanda
				if (bBevandaInPreparazione == 1)
				{
					//è la prima volta che passo di qui
					bBevandaInPreparazione = 2;
					timeStartedMSec = timeNowMSec;

					if (cpuStatus.bShowDialogStopSelezione)
						runningSel.status = eRunningSelStatus_runningCanUseStopBtn;
					else
						runningSel.status = eRunningSelStatus_running;

					//la selezione è in preparazione, mando la notifica al subscriber che ha richiesto la bevanda
					if (runningSel.sub)
						notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
				}

				//timeout di sicurezza. Diamo per scontato che una bevanda non possa durare più di TIMEOUT_SELEZIONE_2_MSEC
				if (timeNowMSec - timeStartedMSec >= TIMEOUT_SELEZIONE_2_MSEC)
				{
					logger->log("priv_handleState_selection() => aborting, TIMEOUT_SELEZIONE_2_MSEC");
					priv_onSelezioneTerminataKO();
					return;
				}
				break;

			case eStatoPreparazioneBevanda_doing_nothing:
				if (bBevandaInPreparazione == 2)
				{
					//tutto ok, selezione terminata!!
					runningSel.status = eRunningSelStatus_finished_OK;
					if (runningSel.sub)
						notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
					priv_enterState_normal();
					return;
				}
				else
				{
					//questa condizione non dovrebbe mai verificarsi, la lascio per ogni evenieneza
					logger->log("priv_handleState_selection() => aborting, unknown status");
					priv_onSelezioneTerminataKO();
					return;
				}
				break;
			}
		}
	}
}

//***************************************************
void Server::priv_onSelezioneTerminataKO()
{
	runningSel.status = eRunningSelStatus_finished_KO;
	if (runningSel.sub)
		notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
	priv_enterState_normal();
}

