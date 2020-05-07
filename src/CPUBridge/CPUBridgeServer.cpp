#include "CPUBridgeServer.h"
#include "CPUBridge.h"
#include "CPUChannelCom.h"
#include "CPUBridgeVersion.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaMemory.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "../rheaCommonLib//rheaUTF16.h"
#include "EVADTSParser.h"

using namespace cpubridge;


//***************************************************
Server::Server()
{
	localAllocator = NULL;
	chToCPU = NULL;
	logger = &nullLogger;

	memset(utf16_CPUMasterVersionString, 0, sizeof(utf16_CPUMasterVersionString));

	runningSel.selNum = 0;
	runningSel.sub = NULL;
	runningSel.stopSelectionWasRequested = 0;
	runningSel.status = eRunningSelStatus_finished_OK;

	showCPUStringModelAndVersionUntil_msec = 0;
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

	localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("cpuBrigeServer");
	subscriberList.setup(localAllocator, 8);

	//apro il canale di comunicazione con la CPU fisica
	bool ret = chToCPU->isOpen();
	if (ret)	
		logger->log("Started OK\n");
	else
		logger->log("Started FAIL\n");
	logger->decIndent();

	//init language
	lang_init(&language);
	lang_open(&language, "GB");

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
	rhea::thread::sleepMSec(500);
	
	//rimuove i subscriber
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
		priv_deleteSubscriber(subscriberList.getElem(i), false);
	subscriberList.unsetup();

	RHEADELETE (rhea::memory_getDefaultAllocator(), localAllocator);
}

//***************************************************
void Server::run()
{
	priv_enterState_compatibilityCheck();

	while (stato.get() != sStato::eStato_quit)
	{
        switch (stato.get())
		{
		case sStato::eStato_compatibilityCheck:
			priv_handleState_compatibilityCheck();
			break;

		case sStato::eStato_CPUNotSupported:
			priv_handleState_CPUNotSupported();
			break;

		case sStato::eStato_DA3_sync:
			priv_handleState_DA3Sync();
			break;

		default:
        case sStato::eStato_comError:
			priv_handleState_comError();
			break;

        case sStato::eStato_normal:
			priv_handleState_normal();
			break;

        case sStato::eStato_selection:
			priv_handleState_selection();
			break;

        case sStato::eStato_programmazione:
			priv_handleState_programmazione();
            break;

		case sStato::eStato_regolazioneAperturaMacina:
			priv_handleState_regolazioneAperturaMacina();
			break;

		case sStato::eStato_telemetry:
			priv_handleState_telemetry();
			break;
		}
	}

	logger->log("Quitting due to CPUBRIDGE_SUBSCRIBER_ASK_DIE...\n");
	for (u8 i = 0; i < 10; i++)
		priv_handleMsgQueues(rhea::getTimeNowMSec(), 50);

	while (1)
	{
		if (!priv_handleMsgQueues(rhea::getTimeNowMSec(), 500))
			break;
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
bool Server::priv_handleMsgQueues(u64 timeNowMSec UNUSED_PARAM, u32 timeOutMSec)
{
	//vediamo se ho dei messaggi in coda
	const u8 nEvent = waitList.wait (timeOutMSec);

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
					if (rhea::event::compare(subscriberList(i2)->hEvent, waitList.getEventSrcAsOSEvent(i)))
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

	return (nEvent > 0);
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
				rhea::thread::pushMsg (hToThreadW, CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER, (u32)CPUBRIDGE_VERSION, &sub->q, sizeof(sSubscriber));

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

/***************************************************
 * Uno dei miei subscriber mi ha inviato una richiesta
 */
void Server::priv_handleMsgFromSingleSubscriber (sSubscription *sub)
{
	rhea::thread::sMsg msg;
	while (rhea::thread::popMsg (sub->q.hFromOtherToCpuR, &msg))
	{
		const u16 handlerID = (u16)msg.paramU32;

		//questo è un comando di comodo per simulare "uno qualunque" dei comandi P.
		//In realtà, è necessario gestire qui caso per caso
		if (msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD)
		{
			if (priv_handleProgrammingMessage(sub, handlerID, msg))
				return;

			eCPUProgrammingCommand cmd;
			const u8 *optionalData;
			cpubridge::translate_CPU_PROGRAMMING_CMD(msg, &cmd, &optionalData);
			switch (cmd)
			{
			case eCPUProgrammingCommand_getTimeNextLavaggioCappuccinatore:
				msg.what = CPUBRIDGE_SUBSCRIBER_ASK_TIME_NEXT_LAVSAN_CAPPUCC;
				break;

			default:
				return;
			}
		}

		switch (msg.what)
		{
		default:
			logger->log("CPUBridgeServer::priv_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_DIE:
			stato.set(sStato::eStato_quit);
			break;

		case CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST:
			priv_deleteSubscriber(sub, true);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION:
		{
			u8 selNumber = 0x00;
			translate_CPU_START_SELECTION(msg, &selNumber);
			priv_enterState_selection(selNumber, sub);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_DA3_SYNC:
			priv_enterState_DA3Sync();
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION:
			if (stato.get() == sStato::eStato_selection)
				runningSel.stopSelectionWasRequested = 1;
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM:
		{
			//Ho optato per inviare subito il comando B invece che schedularlo al prossimo giro
			keepOnSendingThisButtonNum = 0;
			u8 btnToSend = 0;
			translate_CPU_SEND_BUTTON(msg, &btnToSend);

			u8 bufferW[32];
			const u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B(btnToSend, lang_getErrorCode(&language), bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 500))
				priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_KEEP_SENDING_BUTTON_NUM:
		{
			//Da ora in poi, nel comando B invio sempre quest bntNum fino a che non
			//ricevo un analogo msg con btnNum = 0
			translate_CPU_KEEP_SENDING_BUTTON_NUM(msg, &keepOnSendingThisButtonNum);

			u8 bufferW[32];
			const u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B(keepOnSendingThisButtonNum, lang_getErrorCode(&language), bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 500))
				priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);
		}
		break;


		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS:
			notify_CPU_RUNNING_SEL_STATUS(sub->q, handlerID, logger, runningSel.status);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE:
			notify_CPU_FULLSTATE(sub->q, handlerID, logger, &cpuStatus);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM:
		{
			u8 bufferW[32];
			const u8 nBytesToSend = cpubridge::buildMsg_initialParam_C(2, 0, 0, bufferW, sizeof(bufferW));

			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
			{
				priv_parseAnswer_initialParam(answerBuffer, sizeOfAnswerBuffer);
				notify_CPU_INI_PARAM(sub->q, handlerID, logger, &cpuParamIniziali);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL:
			notify_CPU_SEL_AVAIL_CHANGED(sub->q, handlerID, logger, &cpuStatus.selAvailability);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES:
		{
			u8 bufferW[32];
			const u8 nBytesToSend = cpubridge::buildMsg_initialParam_C(2, 0, 0, bufferW, sizeof(bufferW));

			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
			{
				priv_parseAnswer_initialParam(answerBuffer, sizeOfAnswerBuffer);
				const u8 numPrices = NUM_MAX_SELECTIONS;
				notify_CPU_SEL_PRICES_CHANGED(sub->q, handlerID, logger, numPrices, cpu_numDecimalsForPrices, cpuParamIniziali.prices);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE:
			notify_CPU_NEW_LCD_MESSAGE(sub->q, handlerID, logger, &cpuStatus.LCDMsg);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT:
			notify_CPU_CREDIT_CHANGED(sub->q, handlerID, logger, cpuStatus.userCurrentCredit, sizeof(cpuStatus.userCurrentCredit));
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE:
			notify_CPU_STATE_CHANGED(sub->q, handlerID, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT:
			if (stato.get() == sStato::eStato_normal)
				priv_downloadDataAudit(&sub->q, handlerID);
			else
				//rifiuto la richiesta perchè non sono in uno stato valido per la lettura del data audit
				notify_READ_DATA_AUDIT_PROGRESS(sub->q, handlerID, logger, eReadDataFileStatus_finishedKO_cantStart_invalidState, 0, 0);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE:
			if (stato.get() == sStato::eStato_normal)
				priv_downloadVMCDataFile(&sub->q, handlerID);
			else
				//rifiuto la richiesta perchè non sono in uno stato valido per la lettura del file
				notify_READ_VMCDATAFILE_PROGRESS(sub->q, handlerID, logger, eReadDataFileStatus_finishedKO_cantStart_invalidState, 0, 0);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE:
		{
			char srcFullFileNameAndPath[512];
			translate_WRITE_VMCDATAFILE(msg, srcFullFileNameAndPath, sizeof(srcFullFileNameAndPath));
            if (stato.get() == sStato::eStato_normal || stato.get() == sStato::eStato_CPUNotSupported)
				priv_uploadVMCDataFile(&sub->q, handlerID, srcFullFileNameAndPath);
			else
				//rifiuto la richiesta perchè non sono in uno stato valido per la scrittura del file
				notify_WRITE_VMCDATAFILE_PROGRESS(sub->q, handlerID, logger, eWriteDataFileStatus_finishedKO_cantStart_invalidState, 0);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP:
		{
			sCPUVMCDataFileTimeStamp ts;
			if (!priv_askVMCDataFileTimeStampAndWaitAnswer(&ts, 2000))
				ts.setInvalid();
			notify_CPU_VMCDATAFILE_TIMESTAMP(sub->q, handlerID, logger, ts);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_WRITE_CPUFW:
		{
			char srcFullFileNameAndPath[512];
			translate_WRITE_CPUFW(msg, srcFullFileNameAndPath, sizeof(srcFullFileNameAndPath));
			priv_uploadCPUFW(&sub->q, handlerID, srcFullFileNameAndPath);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE:
		{
			u8 block[VMCDATAFILE_BLOCK_SIZE_IN_BYTE];
			u8 blocco_n_di = 0;
			u8 tot_num_blocchi = 0;
			u8 blockNumOffset = 0;
			translate_PARTIAL_WRITE_VMCDATAFILE(msg, block, &blocco_n_di, &tot_num_blocchi, &blockNumOffset);

			u8 bufferW[80];
			const u16 nBytesToSend = cpubridge::buildMsg_writePartialVMCDataFile(block, blocco_n_di, tot_num_blocchi, blockNumOffset, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1500))
			{
				//ok, la CPU ha ricevuto il blocco. A questo punto aggiorno anche il mio da3 file locale
				priv_updateLocalDA3(block, blockNumOffset);

				if (blocco_n_di >= tot_num_blocchi)
				{
					//chiedo alla CPU il nuovo timestamp del file ricevuto e lo salvo localmente
					sCPUVMCDataFileTimeStamp vmcDataFileTimeStamp;
					u8 nRetry = 20;
					while (nRetry--)
					{
						if (priv_askVMCDataFileTimeStampAndWaitAnswer(&vmcDataFileTimeStamp, 2000))
						{
							cpubridge::saveVMCDataFileTimeStamp(vmcDataFileTimeStamp);
							break;
						}
					}
				}

				//aggiorno alcuni dati che conservo anche localmente
				priv_retreiveSomeDataFromLocalDA3();

				//notifico il client
				notify_WRITE_PARTIAL_VMCDATAFILE(sub->q, handlerID, logger, blockNumOffset);
			}
			else
			{
				//la CPU non ha validato il blocco
				notify_WRITE_PARTIAL_VMCDATAFILE(sub->q, handlerID, logger, 0xff);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER:
		{
			eCPUProgrammingCommand_decounter which = eCPUProgrammingCommand_decounter_unknown;
			u16 valore = 0;
			cpubridge::translate_CPU_SET_DECOUNTER(msg, &which, &valore);


			u8 bufferW[32];
			const u16 nBytesToSend = cpubridge::buildMsg_setDecounter(which, valore, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
			{
				which = (eCPUProgrammingCommand_decounter)answerBuffer[4];
				valore = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[5]);
				notify_CPU_DECOUNTER_SET(sub->q, handlerID, logger, which, valore);
			}
			else
				notify_CPU_DECOUNTER_SET(sub->q, handlerID, logger, eCPUProgrammingCommand_decounter_error, 0);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_ALL_DECOUNTER_VALUES:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getAllDecounterValues(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
			{
				u16 decounters[13];
				for (u8 i = 0; i < 13; i++)
					decounters[i] = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[4 + i * 2]);
				notify_CPU_ALL_DECOUNTER_VALUES(sub->q, handlerID, logger, decounters);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_EXTENDED_CONFIG_INFO:
		{
			sExtendedCPUInfo info;
			if (priv_prepareSendMsgAndParseAnswer_getExtendedCOnfgInfo_c(&info))
				notify_EXTENDED_CONFIG_INFO(sub->q, handlerID, logger, &info);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CPU_ATTIVAZIONE_MOTORE:
		{
			u8 motore_1_10, durata_dSec, numRipetizioni, pausaTraRipetizioni_dSec;
			cpubridge::translate_CPU_ATTIVAZIONE_MOTORE(msg, &motore_1_10, &durata_dSec, &numRipetizioni, &pausaTraRipetizioni_dSec);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_attivazioneMotore(motore_1_10, durata_dSec, numRipetizioni, pausaTraRipetizioni_dSec, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
				notify_ATTIVAZIONE_MOTORE(sub->q, handlerID, logger, answerBuffer[4], answerBuffer[5], answerBuffer[6], answerBuffer[7]);
			else
				notify_ATTIVAZIONE_MOTORE(sub->q, handlerID, logger, 0xff, 0, 0, 0);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO:
		{
			u8 macina_1o2 = 0;
			u16 totalePesata_dgram = 0;
			cpubridge::translate_CPU_CALCOLA_IMPULSI_GRUPPO(msg, &macina_1o2, &totalePesata_dgram);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_calcolaImpulsiGruppo(macina_1o2, totalePesata_dgram, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
				notify_CALCOLA_IMPULSI_GRUPPO_STARTED(sub->q, handlerID, logger);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_CALCOLO_IMPULSI_GRUPPO:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getStatoCalcoloImpulsiGruppo(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				u8 stato = answerBuffer[4];
				u16 valore = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[5]);
				notify_STATO_CALCOLO_IMPULSI_GRUPPO(sub->q, handlerID, logger, stato, valore);
			}
			else
				notify_STATO_CALCOLO_IMPULSI_GRUPPO(sub->q, handlerID, logger, 0, 0);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE:
		{
			eCPUProgrammingCommand_motor motore;
			u16 valore;
			cpubridge::translate_CPU_SET_FATTORE_CALIB_MOTORE(msg, &motore, &valore);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_setFattoreCalibMotore(motore, valore, bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				eCPUProgrammingCommand_motor motore = (eCPUProgrammingCommand_motor)answerBuffer[4];
				u16 valore = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[5]);
				notify_SET_FATTORE_CALIB_MOTORE(sub->q, handlerID, logger, motore, valore);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_GRUPPO:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getStatoGruppo(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				eCPUProgrammingCommand_statoGruppo stato;
				switch (answerBuffer[4])
				{
				case 0:		stato = eCPUProgrammingCommand_statoGruppo_nonAttaccato; break;
				default:	stato = eCPUProgrammingCommand_statoGruppo_attaccato; break;
				}
				notify_STATO_GRUPPO(sub->q, handlerID, logger, stato);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_TIME:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getTime(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				const u8 hh = answerBuffer[4];
				const u8 mm = answerBuffer[5];
				const u8 ss = answerBuffer[6];
				if (hh<24 && mm<60 && ss<60)
					notify_GET_TIME(sub->q, handlerID, logger, hh, mm, ss);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_DATE:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getDate(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				const u16 yy = 2000 + answerBuffer[4];
				const u8 mm = answerBuffer[5];
				const u8 dd = answerBuffer[6];
				if ((mm>=1 && mm<=12) && (dd>=1 && dd<=31) && (yy>=2017 && yy<=2050))
					notify_GET_DATE(sub->q, handlerID, logger, yy, mm, dd);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_DATE:
		{
			u16 year = 0;
			u8 month = 0;
			u8 day = 0;
			cpubridge::translate_CPU_SET_DATE(msg, &year, &month, &day);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_setDate(bufferW, sizeof(bufferW), year, month, day);
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				const u16 y = 2000 + answerBuffer[4];
				const u8 m = answerBuffer[5];
				const u8 d = answerBuffer[6];
				notify_SET_DATE(sub->q, handlerID, logger, y, m, d);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_TIME:
		{
			u8 hh = 0;
			u8 mm = 0;
			u8 ss = 0;
			cpubridge::translate_CPU_SET_TIME(msg, &hh, &mm, &ss);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_setTime(bufferW, sizeof(bufferW), hh, mm, ss);
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				const u8 hh = answerBuffer[4];
				const u8 mm = answerBuffer[5];
				const u8 ss = answerBuffer[6];
				notify_SET_TIME(sub->q, handlerID, logger, hh, mm, ss);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA:
		{
			u8 macina_1o2 = 0;
			cpubridge::translate_CPU_GET_POSIZIONE_MACINA(msg, &macina_1o2);

			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getPosizioneMacina(bufferW, sizeof(bufferW), macina_1o2);
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				macina_1o2 = answerBuffer[4];
				if (macina_1o2 == 11) macina_1o2 = 1;
				else if (macina_1o2 == 12) macina_1o2 = 2;
				u16 pos = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[5]);
				notify_CPU_POSIZIONE_MACINA(sub->q, handlerID, logger, macina_1o2, pos);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA:
		{
			u8 macina_1o2 = 0;
			eCPUProgrammingCommand_macinaMove m;
			cpubridge::translate_CPU_SET_MOTORE_MACINA(msg, &macina_1o2, &m);

			if (priv_sendAndHandleSetMotoreMacina(macina_1o2, m))
			{
				macina_1o2 = answerBuffer[4];
				if (macina_1o2 == 11) macina_1o2 = 1;
				else if (macina_1o2 == 12) macina_1o2 = 2;
				notify_CPU_MOTORE_MACINA(sub->q, handlerID, logger, macina_1o2, (eCPUProgrammingCommand_macinaMove)answerBuffer[5]);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA:
		{
			u8 macina_1o2 = 0;
			u16 target = 0;
			cpubridge::translate_CPU_SET_POSIZIONE_MACINA(msg, &macina_1o2, &target);
			//priv_setPosMacina(sub->q, handlerID, macina_1o2, target);
			priv_enterState_regolazioneAperturaMacina(macina_1o2, target);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE:
		{
			u8 selNum = 0;
			eCPUProgrammingCommand_testSelectionDevice m;
			cpubridge::translate_CPU_TEST_SELECTION(msg, &selNum, &m);
			
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_testSelection(bufferW, sizeof(bufferW), selNum, m);
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
			{
				selNum = answerBuffer[4];
				m = (eCPUProgrammingCommand_testSelectionDevice)answerBuffer[5];
				notify_CPU_TEST_SELECTION(sub->q, handlerID, logger, selNum, m);
			}
			else
				notify_CPU_TEST_SELECTION(sub->q, handlerID, logger, 0xff, eCPUProgrammingCommand_testSelectionDevice_unknown);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_NOMI_LINGUE_CPU:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getNomiLingueCPU(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
			{
				u16 strLingua1UTF16[33];
				u16 strLingua2UTF16[33];
				memset(strLingua1UTF16, 0, sizeof(strLingua1UTF16));
				memset(strLingua2UTF16, 0, sizeof(strLingua2UTF16));

				u8 z = 5;
				if (answerBuffer[4] == 0x01)
				{
					//caso unicode
					for (u8 i = 0; i < 32;i++)
					{
						strLingua1UTF16[i] = (u16)answerBuffer[z] + (u16)answerBuffer[z + 1] * 256;
						z += 2;
					}
					for (u8 i = 0; i < 32;i++)
					{
						strLingua2UTF16[i] = (u16)answerBuffer[z] + (u16)answerBuffer[z + 1] * 256;
						z += 2;
					}
				}
				else
				{
					for (u8 i = 0; i < 32;i++)
						strLingua1UTF16[i] = (u16)answerBuffer[z++];
					for (u8 i = 0; i < 32;i++)
						strLingua2UTF16[i] = (u16)answerBuffer[z++];
				}
				
				notify_NOMI_LINGE_CPU(sub->q, handlerID, logger, strLingua1UTF16, strLingua2UTF16);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_DISINSTALLAZIONE:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_disintallazione(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_RICARICA_FASCIA_ORARIA_FV:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_ricaricaFasciaOrariaFreevend(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_PARTIALDATA:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_EVAresetPartial(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
					notify_EVA_RESET_PARTIALDATA(sub->q, handlerID, logger, true);
				else
					notify_EVA_RESET_PARTIALDATA(sub->q, handlerID, logger, false);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_TOTALS:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_EVAresetTotals(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
					notify_CPU_EVA_RESET_TOTALS(sub->q, handlerID, logger);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_VOLT_AND_TEMP:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_getVoltAndTemp(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
				{
					const u8 tCamera = answerBuffer[4];
					const u8 tBollitore = answerBuffer[5];
					const u8 tCappuccinatore = answerBuffer[6];
					const u16 voltaggio = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[7]);
					notify_GET_VOLT_AND_TEMP(sub->q, handlerID, logger, tCamera, tBollitore, tCappuccinatore, voltaggio);
				}
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_OFF_REPORT:
			{
				u8 indexNum = 0;
				cpubridge::translate_CPU_GET_OFF_REPORT(msg, &indexNum);

				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_getCPUOFFReportDetails(bufferW, sizeof(bufferW), indexNum);
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
				{
					const u8 lastIndexNum = answerBuffer[5];
					const u8 numOffs = (u8) ((sizeOfAnswerBuffer - 7) / 8);
					sCPUOffSingleEvent offs[32];
					u16 ct = 6;
					for (u8 i = 0; i < numOffs; i++)
					{
                        offs[i].codice = answerBuffer[ct++];
                        const u8 tipo = answerBuffer[ct++];
						if ((tipo >= 'a' && tipo <= 'z') || (tipo >= 'A' && tipo <= 'Z') || (tipo >= '0' && tipo <= '9'))
							offs[i].tipo = tipo;
						else
							offs[i].tipo = ' ';
                        offs[i].ora = answerBuffer[ct++];
                        offs[i].minuto = answerBuffer[ct++];
                        offs[i].giorno = answerBuffer[ct++];
                        offs[i].mese = answerBuffer[ct++];
                        offs[i].anno = answerBuffer[ct++];
                        offs[i].stato = answerBuffer[ct++];
					}

                    notify_GET_OFF_REPORT(sub->q, handlerID, logger, indexNum, lastIndexNum, offs, numOffs);
				}
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_FLUX_INFORMATION:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_getLastFluxInformation(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
				{
					const u16 lastFlux = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[4]);
					const u16 lastGrinderPos = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[6]);
					notify_GET_LAST_FLUX_INFORMATION(sub->q, handlerID, logger, lastFlux, lastGrinderPos);
				}
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_SHOW_STR_VERSION_AND_MODEL:
		case CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_STR_VERSION_AND_MODEL:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_getCPUStringVersionAndModel(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
				{
					if (sizeOfAnswerBuffer > 30)
					{
						memset(utf16_CPUMasterVersionString, 0, sizeof(utf16_CPUMasterVersionString));
						if (answerBuffer[4] == 1)
						{
							//CPU ha mandato una stringa in unicode
							memcpy(utf16_CPUMasterVersionString, &answerBuffer[5], 64);
						}
						else
						{
							//CPU ha mandato una stringa in ascii
							for (u8 i = 0; i < 32; i++)
								utf16_CPUMasterVersionString[i] = (u16)answerBuffer[5 + i];
						}

						rhea::utf16::rtrim(utf16_CPUMasterVersionString);

						if (msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SHOW_STR_VERSION_AND_MODEL)
						{
							//per i prossimi 7 secondi, nella risposta al comando B inserirò di default il messaggio con la versione e modello
							showCPUStringModelAndVersionUntil_msec = rhea::getTimeNowMSec() + 7000;
						}
						else
						{
							notify_CPU_STRING_VERSION_AND_MODEL(sub->q, handlerID, logger, utf16_CPUMasterVersionString);
						}
					}
				}
			}
			break;

		
		case CPUBRIDGE_SUBSCRIBER_ASK_START_MODEM_TEST:
			{
				u8 bufferW[16];
				const u16 nBytesToSend = cpubridge::buildMsg_startModemTest(bufferW, sizeof(bufferW));
				u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
				if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
					notify_CPU_START_MODEM_TEST(sub->q, handlerID, logger);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_TIME_NEXT_LAVSAN_CAPPUCC:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getTimeNextLavaggioSanCappuccinatore(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 4000))
			{
				if (sizeOfAnswerBuffer == 7)
				{
					const u8 hh = answerBuffer[4];
					const u8 mm = answerBuffer[5];
					if (hh < 24 && mm < 60)
						notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(sub->q, handlerID, logger, hh, mm);
				}
			}
		}
		break;


		case CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_GRUPPO:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_startTestAssorbimentoGruppo(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
				notify_START_TEST_ASSORBIMENTO_GRUPPO(sub->q, handlerID, logger);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_GRUPPO:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getStatoTestAssorbimentoGruppo(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
			{
				//[fase] [esito] [report1up LSB MSB]  [report1down LSB MSB] ... [report6up LSB MSB] [report6down LSB MSB]
				const u8 fase = answerBuffer[4];
				const u8 esito = answerBuffer[5];
				u16 results[12];
				for (u8 i=0; i<12; i++)
					results[i] = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[6+2*i]);
				notify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(sub->q, handlerID, logger, fase, esito, results);
			}
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_MOTORIDUTTORE:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_startTestAssorbimentoMotoriduttore(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
				notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(sub->q, handlerID, logger);
		}
		break;

		case CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE:
		{
			u8 bufferW[16];
			const u16 nBytesToSend = cpubridge::buildMsg_getStatoTestAssorbimentoMotoriduttore(bufferW, sizeof(bufferW));
			u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
			if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
			{
				//[fase] [esito] [report_up LSB MSB]  [report_down LSB MSB]
				const u8 fase = answerBuffer[4];
				const u8 esito = answerBuffer[5];
				const u16 reportUP = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[6]);
				const u16 reportDOWN = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[8]);
				notify_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(sub->q, handlerID, logger, fase, esito, reportUP, reportDOWN);
			}
		}
		break;
		} //switch
	} //while
}


//**********************************************
bool Server::priv_prepareSendMsgAndParseAnswer_getExtendedCOnfgInfo_c(sExtendedCPUInfo *out)
{
	u8 bufferW[16];
	const u16 nBytesToSend = cpubridge::buildMsg_getExtendedConfigInfo(bufferW, sizeof(bufferW));
	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1000))
		return false;

	//parsing della risposta
	out->msgVersion = answerBuffer[3];

	//è necessario che la versione del msg sia la 2, altrimenti fingo che la CPU non mi abbia nemmeno risposto
	if (out->msgVersion != 2)
		return false;

	switch (answerBuffer[4])
	{
	default: 
		out->machineType = eCPUMachineType_unknown; 
		break;

	case 0x00: 
		out->machineType = eCPUMachineType_instant; 
		break;

	case 0x01:
        out->machineType = eCPUMachineType_espresso1;
        break;
        
	case 0x02:
		out->machineType = eCPUMachineType_espresso2;
		break;
	}
	out->machineModel = answerBuffer[5];
	out->isInduzione = answerBuffer[6];
	return true;
}

//**********************************************
void Server::priv_updateLocalDA3 (const u8 *blockOf64Bytes, u8 blockNum) const
{
	char s[256];
	sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
	
	rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
	u32 fileSize = 0;
	u8 *buffer = rhea::fs::fileCopyInMemory(s, allocator, &fileSize);
	if (NULL == buffer)
		return;

	const u32 blockPosition = VMCDATAFILE_BLOCK_SIZE_IN_BYTE * blockNum;
	if (fileSize >= blockPosition + VMCDATAFILE_BLOCK_SIZE_IN_BYTE)
	{
		memcpy(&buffer[blockPosition], blockOf64Bytes, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);

		const u32 SIZE = 1024;
		FILE *f = fopen(s, "wb");
		u32 ct = 0;
		while (fileSize >= SIZE)
		{
			fwrite(&buffer[ct], SIZE, 1, f);
			ct += SIZE;
			fileSize -= SIZE;
		}
		if (fileSize)
			fwrite(&buffer[ct], fileSize, 1, f);
		fclose(f);
	}
	RHEAFREE(allocator, buffer);

    //salvo data e ora dell'ultima modifica
    rhea::DateTime dt;
    dt.setNow();
    sprintf_s(s, sizeof(s), "%s/last_installed/da3/dateUM.bin", rhea::getPhysicalPathToAppFolder());
    FILE *f = fopen(s, "wb");
    u64 u = dt.getInternalRappresentation();
    fwrite (&u, sizeof(u64), 1, f);
    fclose(f);
}

//**********************************************
bool Server::priv_handleProgrammingMessage (sSubscription *sub, u16 handlerID, const rhea::thread::sMsg &msg)
{
	eCPUProgrammingCommand cmd;
	const u8 *optionalData;
	cpubridge::translate_CPU_PROGRAMMING_CMD (msg, &cmd, &optionalData);


	u8 bufferW[32];
	u8 nBytesToSend = 0;
	
	switch (cmd)
	{
	default:
		return false;
		break;

	case eCPUProgrammingCommand_enterProg:
		//il comando "vai in modalità prog" lo mando solo se la CPU non è già in modalità PROG
		if (cpuStatus.VMCstate != eVMCState_PROGRAMMAZIONE)
			nBytesToSend = cpubridge::buildMsg_Programming(cmd, NULL, 0, bufferW, sizeof(bufferW));
		break;


	case eCPUProgrammingCommand_querySanWashingStatus:
		nBytesToSend = cpubridge::buildMsg_Programming (cmd, NULL, 0, bufferW, sizeof(bufferW));
		break;

	case eCPUProgrammingCommand_cleaning:
		//in [optionalData] c'è un byte che indica il tipo di lavaggio
		nBytesToSend = cpubridge::buildMsg_Programming(cmd, optionalData, 1, bufferW, sizeof(bufferW));
		break;
	}


	if (nBytesToSend)
	{
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 1500))
		{
			logger->log("ERR sending P[%d] command to CPU\n", cmd);
		}
		else
		{
			//in generale, non mi interessa la risposta della CPU ai comandi prog, a parte per alcune eccezioni.
			//La cpu risponde sempre con:
			// [#] [P] [len] [subcommand] [optional_data] [ck]
			switch (answerBuffer[3])
			{
			case eCPUProgrammingCommand_querySanWashingStatus:
				//la CPU risponde con 3 bytes che indicano:
				//	b0 => fase del lavaggio
				//	b1 => se != da 0 allora vuol dire che la CPU è in attesa della pressione del tasto b-esimo
				//	b2 => come sopra (in pratica la CPU può essere in attesa della pressione del tasto b1 oppure del tasto b2
				//In ogni caso, io ignoro queste cose, mi limito a notificare i miei client
				if (NULL != sub && answerBuffer[2] >= 8)
					notify_SAN_WASHING_STATUS(sub->q, handlerID, logger, answerBuffer[4], answerBuffer[5], answerBuffer[6]);
				break;
			}
		}
	}
	return true;
}

//**********************************************
u8 Server::priv_2DigitHexToInt(const u8 *buffer, u32 index) const
{
	u32 ret = 0;
	rhea::string::convert::hexToInt((const char*)&buffer[index], &ret, 2);
	return (u8)ret;
}

/**********************************************
 * copiato da codice originale e lievemente ripulito, sorry.
 *	Ritorna 0 se tutto OK
 */
bool Server::priv_WriteByteMasterNext (u8 dato_8, bool isLastFlag, u8 *out_bufferW, u32 &in_out_bufferCT) const
{
	if (!isLastFlag)
	{
		out_bufferW[in_out_bufferCT++] = dato_8;
		if (in_out_bufferCT != CPUFW_BLOCK_SIZE)
			return true;
	}

	chToCPU->sendOnlyAndDoNotWait(out_bufferW, in_out_bufferCT, logger);

	u8 checksum_counter = 0;
	for (u32 i = 0; i < in_out_bufferCT; i++)
		checksum_counter += out_bufferW[i];

	if (isLastFlag) 
		return true;

	if (!chToCPU->waitForASpecificChar(checksum_counter, 90))
		return false;

	in_out_bufferCT = 0;
	return true;
}


/***************************************************
 * Dato un file mhx localmente accessibile, lo usa per sovrascrivere il FW della CPU
 * Periodicamente emette un notify_WRITE_CPUFW_PROGRESS() con lo stato dell'upload.
 * Al termine dell'upload, se tutto è andato bene, il mhx sorgente viene copiato pari pari (compreso il suo nome originale) in
 *	app/last_installed/cpu/nomeFileSrc.mhx
 */
eWriteCPUFWFileStatus Server::priv_uploadCPUFW(cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath)
{
	char cpuFWFileNameOnly[256];
	char tempFilePathAndName[512];
	rhea::fs::extractFileNameWithExt(srcFullFileNameAndPath, cpuFWFileNameOnly, sizeof(cpuFWFileNameOnly));

	if (strncmp(srcFullFileNameAndPath, "APP:/temp/", 10) == 0)
	{
		//il file si suppone già esistere nella mia cartella temp
		sprintf_s(tempFilePathAndName, sizeof(tempFilePathAndName), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), cpuFWFileNameOnly);
	}
	else
	{
		//copio il file nella mia directory locale temp
		sprintf_s(tempFilePathAndName, sizeof(tempFilePathAndName), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), cpuFWFileNameOnly);
		if (!rhea::fs::fileCopy(srcFullFileNameAndPath, tempFilePathAndName))
		{
			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_unableToCopyFile, 0);
			return eWriteCPUFWFileStatus_finishedKO_unableToCopyFile;
		}
	}


	if (!rhea::fs::fileExists(tempFilePathAndName))
	{
		if (NULL != subscriber)
			notify_WRITE_CPUFW_PROGRESS (*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_unableToOpenLocalFile, 0);
		return eWriteCPUFWFileStatus_finishedKO_unableToOpenLocalFile;
	}


	//invalido il timestamp del mio da3 locale in modo che al termine dell'operazione di upload
	//del FW CPU, la GPU attivi una fase di sincronizzazione da3
	{
		sCPUVMCDataFileTimeStamp localDA3TimeStamp;
		localDA3TimeStamp.setInvalid();
		saveVMCDataFileTimeStamp(localDA3TimeStamp);
	}


	//per prima cosa si fa fare il reboot alla CPU
	u8 bufferW[CPUFW_BLOCK_SIZE+16];
	u8 nBytesToSend = cpubridge::buildMsg_restart_U(bufferW, 64);
	chToCPU->sendOnlyAndDoNotWait(bufferW, nBytesToSend, logger);
	//chToCPU->closeAndReopen();

	//aspetto di leggere 'k' dal canale
    if (!chToCPU->waitForASpecificChar('k', 10000))
	{
		if (NULL != subscriber)
			notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_k_notReceived, 0);
		return eWriteCPUFWFileStatus_finishedKO_k_notReceived;
	}

	bufferW[0] = 'k';
	chToCPU->sendOnlyAndDoNotWait(bufferW, 1, logger);

	bufferW[0] = 'M';
	chToCPU->sendOnlyAndDoNotWait(bufferW, 1, logger);

	//aspetto di leggere 'M' dal canale
	if (!chToCPU->waitForASpecificChar('M', 10000))
	{
		if (NULL != subscriber)
			notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_M_notReceived, 0);
		return eWriteCPUFWFileStatus_finishedKO_M_notReceived;
	}

	//erasing flash
	if (NULL != subscriber)
		notify_WRITE_CPUFW_PROGRESS(*subscriber, 0, logger, eWriteCPUFWFileStatus_inProgress_erasingFlash, 0);

	char s[512];
	sprintf_s(s, sizeof(s), "%s/last_installed/cpu", rhea::getPhysicalPathToAppFolder());
	rhea::fs::deleteAllFileInFolderRecursively(s, false);


	//aspetto di leggere 'h' dal canale
	if (!chToCPU->waitForASpecificChar('h', 15000))
	{
		if (NULL != subscriber)
			notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_h_notReceived, 0);
		return eWriteCPUFWFileStatus_finishedKO_h_notReceived;
	}



	//Carico il file mhx in memoria
	rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
	u32 fsize = 0;
	u8 *fileInMemory = rhea::fs::fileCopyInMemory(tempFilePathAndName, allocator, &fsize);
	

	//inizio scrittura flash
    u32 ct = 0;
	u8 recType = 0;
	u32 bufferWCT = 0;
	u16 lastKbWriteSent = u16MAX;
	u16 kbWrittenSoFar = 0;
	eWriteCPUFWFileStatus ret = eWriteCPUFWFileStatus_finishedOK;
	do
	{
		if (fileInMemory[ct] != 'S')
		{
			ret = eWriteCPUFWFileStatus_finishedKO_generalError;
			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, ret, 5);
			break;
		}

		recType = fileInMemory[ct + 1];
        const u8 numByte = priv_2DigitHexToInt(fileInMemory, ct + 2);
		ct += 4;

		if (recType == '0' || recType == '3' || recType == '8' || recType == '9' || numByte == 4)
		{
			ct += (numByte + 1) * 2;
			continue;
		}

		if (!priv_WriteByteMasterNext(numByte, false, bufferW, bufferWCT))
		{
			ret = eWriteCPUFWFileStatus_finishedKO_generalError;
			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, ret, 6);
			break;
		}

		for (u8 i = 0; i < (numByte - 1); i++)
		{
			const u8 dato8 = priv_2DigitHexToInt(fileInMemory, ct);
			ct += 2;
			if (!priv_WriteByteMasterNext(dato8, false, bufferW, bufferWCT))
			{
				ret = eWriteCPUFWFileStatus_finishedKO_generalError;
				if (NULL != subscriber)
					notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, ret, 7);
				break;
			}
		}
		ct += 4;

		//notifico che ho letto un altro Kb
		kbWrittenSoFar = (u16)(ct >> 10);
		if (kbWrittenSoFar != lastKbWriteSent)
		{
			lastKbWriteSent = kbWrittenSoFar;

			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, 0, logger, eWriteCPUFWFileStatus_inProgress, kbWrittenSoFar);
		}

	} while (recType < '7');
	
	if (ret == eWriteCPUFWFileStatus_finishedOK)
	{
		priv_WriteByteMasterNext(4, false, bufferW, bufferWCT);
		priv_WriteByteMasterNext(0, true, bufferW, bufferWCT);

		u8 ck = 0;
		for (u32 i = 0; i < bufferWCT; i++)
			ck += bufferW[i];
		if (!chToCPU->waitForASpecificChar(ck, 90))
		{
			ret = eWriteCPUFWFileStatus_finishedKO_generalError;
			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedKO_generalError, 8);
		}
		else	
		{
			if (NULL != subscriber)
				notify_WRITE_CPUFW_PROGRESS(*subscriber, handlerID, logger, eWriteCPUFWFileStatus_finishedOK, kbWrittenSoFar);
		}
	}
	RHEAFREE(allocator, fileInMemory);

	//in caso di successo, copio il file mhx nella mia cartella app/last_installed/cpu/nomeFileSrc.mhx
	if (ret == eWriteCPUFWFileStatus_finishedOK)
	{
		sprintf_s(s, sizeof(s), "%s/last_installed/cpu/%s", rhea::getPhysicalPathToAppFolder(), cpuFWFileNameOnly);
		rhea::fs::fileCopy(tempFilePathAndName, s);
		rhea::fs::fileDelete(tempFilePathAndName);

        //reboot CPU
        nBytesToSend = cpubridge::buildMsg_restart_U(bufferW, 64);
        chToCPU->sendOnlyAndDoNotWait(bufferW, nBytesToSend, logger);
	}
	return ret;
}


/***************************************************
 * Dato un file da3 localmente accessibile, lo invia alla CPU.
 * Periodicamente emette un notify_WRITE_VMCDATAFILE_PROGRESS() con lo stato dell'upload.
 * Al termine dell'upload, se tutto è andatao bene, il da3 sorgente viene copiato pari pari (compreso il suo nome originale) in 
 *	app/last_installed/da3/nomeFileSrc.da3 e anche in  app/current/da3/vmcDataFile.da3
 * Al termine della copia, chiede a CPU il timestamp del da3 e lo salva in current/da3/vmcDataFile.timestamp
 */
eWriteDataFileStatus Server::priv_uploadVMCDataFile (cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath)
{
	char fileName[256];
	char tempFilePathAndName[512];
	rhea::fs::extractFileNameWithExt(srcFullFileNameAndPath, fileName, sizeof(fileName));

	if (strncmp(srcFullFileNameAndPath, "APP:/temp/", 10) == 0)
	{
		//il file si suppone già esistere nella mia cartella temp
		sprintf_s(tempFilePathAndName, sizeof(tempFilePathAndName), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), fileName);
	}
	else
	{
		//copio il file nella mia directory locale temp
		sprintf_s(tempFilePathAndName, sizeof(tempFilePathAndName), "%s/temp/%s", rhea::getPhysicalPathToAppFolder(), fileName);
		if (!rhea::fs::fileCopy(srcFullFileNameAndPath, tempFilePathAndName))
		{
			notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedKO_unableToCopyFile, 0);
			return eWriteDataFileStatus_finishedKO_unableToCopyFile;
		}
	}

	FILE *f = fopen(tempFilePathAndName, "rb");
	if (NULL == f)
	{
		notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedKO_unableToOpenLocalFile, 0);
		return eWriteDataFileStatus_finishedKO_unableToOpenLocalFile;
	}

	//il meccanismo attuale prevedere che la io mandi pacchetti da 64b di dati alla CPU fino al raggiungimento della
	//dimensione totale del file. Il tutto è hard-codato...
	u32 bytesWrittenSoFar = 0;
	u16 kbWrittenSoFar = 0;
	u16 lastKbWriteSent = u16MAX;
	const u8 TOT_NUM_BLOCKS = VMCDATAFILE_TOTAL_FILE_SIZE_IN_BYTE / VMCDATAFILE_BLOCK_SIZE_IN_BYTE;
	for (u8 blockNum=0; blockNum < TOT_NUM_BLOCKS; blockNum++)
	{
		//leggo da file
		u8 fileBuffer[VMCDATAFILE_BLOCK_SIZE_IN_BYTE];
		fread(fileBuffer, VMCDATAFILE_BLOCK_SIZE_IN_BYTE, 1, f);

		//creo il msg da mandare a CPU
		u8 bufferCPUMsg[VMCDATAFILE_BLOCK_SIZE_IN_BYTE +32];
		const u16 nBytesToSend = buildMsg_writeVMCDataFile(fileBuffer, blockNum, TOT_NUM_BLOCKS, bufferCPUMsg, sizeof(bufferCPUMsg));
		
		//invio
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (!chToCPU->sendAndWaitAnswer(bufferCPUMsg, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
		{
			//errore, la CPU non ha risposto, abortisco l'operazione
			if (NULL != subscriber)
				notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedKO_cpuDidNotAnswer, kbWrittenSoFar);
			fclose(f);
			return eWriteDataFileStatus_finishedKO_cpuDidNotAnswer;
		}

		//aggiorno contatore di byte scritti
		bytesWrittenSoFar += VMCDATAFILE_BLOCK_SIZE_IN_BYTE;
		kbWrittenSoFar = (u16)(bytesWrittenSoFar >> 10);

		//notifico che ho letto un altro Kb
		if (kbWrittenSoFar != lastKbWriteSent)
		{
			lastKbWriteSent = kbWrittenSoFar;

			if (NULL != subscriber)
				notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, 0, logger, eWriteDataFileStatus_inProgress, kbWrittenSoFar);
		}
	}

	//finito!
	fclose(f);
	assert(bytesWrittenSoFar >= VMCDATAFILE_TOTAL_FILE_SIZE_IN_BYTE);

	//dopo qualche esperimento, ho notato che a seguito di un upload, la CPU cmq fa delle modifiche al DA3.
	//Chiedo quindi quei blocchi che la CPU ha modificato
	{
		u32 sizeOfBuffer= 0;
		u8 *tempBuffer = rhea::fs::fileCopyInMemory(tempFilePathAndName, localAllocator, &sizeOfBuffer);

		u16 block = 151;
		if (!priv_prepareAndSendMsg_readVMCDataFileBlock(block))
		{
			if (NULL != subscriber)
				notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedKO_cpuDidNotAnswer2, kbWrittenSoFar);
			return eWriteDataFileStatus_finishedKO_cpuDidNotAnswer2;
		}
		memcpy (&tempBuffer[block*VMCDATAFILE_BLOCK_SIZE_IN_BYTE], &answerBuffer[5], VMCDATAFILE_BLOCK_SIZE_IN_BYTE);

		//sovrascrivo il file con le info
		f = fopen(tempFilePathAndName, "wb");
		fwrite (tempBuffer, sizeOfBuffer, 1, f);
		fclose(f);

		RHEAFREE(localAllocator, tempBuffer);
	}



	//a questo punto, copio il file temporaneo nella mia cartella last_installed e nella cartella current (qui gli cambio anche il nome con quello di default)
	char s[512];
	sprintf_s(s, sizeof(s), "%s/last_installed/da3", rhea::getPhysicalPathToAppFolder());
	rhea::fs::deleteAllFileInFolderRecursively(s, false);

	sprintf_s(s, sizeof(s), "%s/last_installed/da3/%s", rhea::getPhysicalPathToAppFolder(), fileName);
	rhea::fs::fileCopy(tempFilePathAndName, s);

    sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
	rhea::fs::fileCopy(tempFilePathAndName, s);

	rhea::fs::fileDelete(tempFilePathAndName);


	//chiedo alla CPU il nuovo timestamp del file ricevuto e lo salvo localmente
	sCPUVMCDataFileTimeStamp	vmcDataFileTimeStamp;
    u8 nRetry = 20;
    while (nRetry--)
    {
        if (priv_askVMCDataFileTimeStampAndWaitAnswer(&vmcDataFileTimeStamp, 2000))
        {
            cpubridge::saveVMCDataFileTimeStamp(vmcDataFileTimeStamp);
            break;
        }
    }

    //salvo data e ora dell'ultima modifica
    rhea::DateTime dt;
    dt.setNow();
    sprintf_s(s, sizeof(s), "%s/last_installed/da3/dateUM.bin", rhea::getPhysicalPathToAppFolder());
    f = fopen(s, "wb");
    u64 u = dt.getInternalRappresentation();
    fwrite (&u, sizeof(u64), 1, f);
    fclose(f);

	//aggiorno alcuni dati che conservo in memoria
	priv_retreiveSomeDataFromLocalDA3();

	//notifico il client e finisco
	if (NULL != subscriber)
		notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedOK, kbWrittenSoFar);

	return eWriteDataFileStatus_finishedOK;
}

//***************************************************
u16 Server::priv_prepareAndSendMsg_readVMCDataFileBlock (u16 blockNum)
{
	u8 bufferW[32];
	const u16 nBytesToSend = buildMsg_readVMCDataFile((u8)blockNum, bufferW, sizeof(bufferW));

	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
		return 0;
	return sizeOfAnswerBuffer;
}

/***************************************************
 * Chiede alla CPU il file di da3 e lo salva localmente in app/temp/vmcDataFile%d.da3
 * [%d] è un progressivo che viene comunicato alla fine del download nel messaggio di conferma (e messo anche in out_fileID in caso di successo)
 * Periodicamente notifica emettendo un messaggio notify_READ_VMCDATAFILE_PROGRESS() che contiene lo stato di avanzamento
 * del download
 */
eReadDataFileStatus Server::priv_downloadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID, u16 *out_fileID)
{
	//il file che scarico dalla CPU lo salvo localmente con un nome ben preciso
	char fullFilePathAndName[256];
	u16 fileID = 0;
	while (1)
	{
		sprintf_s(fullFilePathAndName, sizeof(fullFilePathAndName), "%s/temp/vmcDataFile%d.da3", rhea::getPhysicalPathToAppFolder(), fileID);
		if (!rhea::fs::fileExists(fullFilePathAndName))
			break;
		fileID++;
	}

	FILE *f = fopen(fullFilePathAndName, "wb");
	if (NULL == f)
	{
		notify_READ_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedKO_unableToCreateFile, 0, fileID);
		return eReadDataFileStatus_finishedKO_unableToCreateFile;
	}

	//il meccanismo attuale prevedere che la CPU mandi pacchetti da 64b di dati fino al raggiungimento della
	//dimensione totale del file. Il tutto è hard-codato...
	u32 nPacketSoFar = 0;
	u32 bytesReadSoFar = 0;
	u16 kbReadSoFar = 0;
	u16 lastKbReadSent = u16MAX;
	while (1)
	{
		/*u16 nBytesToSend = buildMsg_readVMCDataFile(nPacketSoFar++, bufferW, sizeof(bufferW));
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
		*/

		u16 sizeOfAnswerBuffer = priv_prepareAndSendMsg_readVMCDataFileBlock(nPacketSoFar++);
		if (0 == sizeOfAnswerBuffer)
		{
			//errore, la CPU non ha risposto, abortisco l'operazione
			if (NULL == subscriber)
			{
				for (u32 i = 0; i < subscriberList.getNElem(); i++)
					notify_READ_VMCDATAFILE_PROGRESS(subscriberList(i)->q, 0, logger, eReadDataFileStatus_finishedKO_cpuDidNotAnswer, kbReadSoFar, fileID);
			}
			else
				notify_READ_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedKO_cpuDidNotAnswer, kbReadSoFar, fileID);

			fclose(f);
			return eReadDataFileStatus_finishedKO_cpuDidNotAnswer;
		}

		//scrivo su file i dati ricevuti
		const u8 payloadLen = sizeOfAnswerBuffer - 6;
		const u8* payload = &answerBuffer[5];
		fwrite(payload, payloadLen, 1, f);

		//aggiorno contatore di byte letti
		bytesReadSoFar += payloadLen;
		kbReadSoFar = (u16)(bytesReadSoFar >> 10);


		if (bytesReadSoFar >= VMCDATAFILE_TOTAL_FILE_SIZE_IN_BYTE)
		{
			//finito!
			if (NULL != out_fileID )
				*out_fileID = fileID;
			if (NULL == subscriber)
			{
				for (u32 i = 0; i < subscriberList.getNElem(); i++)
					notify_READ_VMCDATAFILE_PROGRESS(subscriberList(i)->q, 0, logger, eReadDataFileStatus_finishedOK, kbReadSoFar, fileID);
			}
			else
				notify_READ_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedOK, kbReadSoFar, fileID);

			fclose(f);
			return eReadDataFileStatus_finishedOK;
		}

		//notifico che ho letto un altro Kb
		if (kbReadSoFar != lastKbReadSent)
		{
			lastKbReadSent = kbReadSoFar;
            priv_handleMsgQueues(rhea::getTimeNowMSec(), 1);

			if (NULL == subscriber)
			{
				for (u32 i = 0; i < subscriberList.getNElem(); i++)
					notify_READ_VMCDATAFILE_PROGRESS(subscriberList(i)->q, 0, logger, eReadDataFileStatus_inProgress, kbReadSoFar, fileID);
			}
			else
				notify_READ_VMCDATAFILE_PROGRESS(*subscriber, 0, logger, eReadDataFileStatus_inProgress, kbReadSoFar, fileID);
		}
	}
}

/***************************************************
 * Chiede alla CPU il file di data-audit e lo salva localmente in app/temp/dataAudit%d.txt
 * [%d] è un progressivo che viene comunicato alla fine del download nel messaggio di conferma.
 * Periodicamente notifica emettendo un messaggio notify_READ_DATA_AUDIT_PROGRESS() che contiene lo stato di avanzamento
 * del download
 */
eReadDataFileStatus Server::priv_downloadDataAudit (cpubridge::sSubscriber *subscriber,u16 handlerID)
{
	//il file che scarico dalla CPU lo salvo localmente con un nome ben preciso
	char fullFilePathAndName[256];
	u16 fileID = 0;
	while (1)
	{
		sprintf_s(fullFilePathAndName, sizeof(fullFilePathAndName), "%s/temp/dataAudit%d.txt", rhea::getPhysicalPathToAppFolder(), fileID);
		if (!rhea::fs::fileExists(fullFilePathAndName))
			break;
		fileID++;
	}


#ifdef _DEBUG
	//hack per velocizzare i test
	{
		char debug_src_eva[256];
		sprintf_s(debug_src_eva, sizeof(debug_src_eva), "%s/last_installed/eva_test.log", rhea::getPhysicalPathToAppFolder());
		rhea::fs::fileCopy(debug_src_eva, fullFilePathAndName);
		Server::priv_downloadDataAudit_onFinishedOK(fullFilePathAndName, fileID);
		if (NULL != subscriber)
			notify_READ_DATA_AUDIT_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedOK, 15, fileID);

		return eReadDataFileStatus_finishedOK;
	}
#endif

	FILE *f = fopen(fullFilePathAndName, "wb");
	if (NULL == f)
	{
		if (NULL != subscriber)
			notify_READ_DATA_AUDIT_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedKO_unableToCreateFile, 0, fileID);
		return eReadDataFileStatus_finishedKO_unableToCreateFile;
	}


    u8 bufferW[32];
    const u16 nBytesToSend = cpubridge::buildMsg_readDataAudit(bufferW, sizeof(bufferW));

    u32 bytesReadSoFar = 0;
    u16 kbReadSoFar = 0;
    u16 lastKbReadSent = u16MAX;
    while (1)
    {
        u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
        if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
        {
            //errore, la CPU non ha risposto, abortisco l'operazione
            if (NULL != subscriber)
                notify_READ_DATA_AUDIT_PROGRESS (*subscriber, handlerID, logger, eReadDataFileStatus_finishedKO_cpuDidNotAnswer, kbReadSoFar, fileID);
            fclose(f);
            return eReadDataFileStatus_finishedKO_cpuDidNotAnswer;
        }

        //scrivo su file i dati ricevuti
        const u8 payloadLen = sizeOfAnswerBuffer - 6;
        const u8* payload = &answerBuffer[5];
        fwrite (payload, payloadLen, 1, f);

        //aggiorno contatore di byte letti
        bytesReadSoFar += payloadLen;
        kbReadSoFar = (u16)(bytesReadSoFar >> 10);


        if (answerBuffer[3] != 0)
        {
            //finito!
			fclose(f);
	
			priv_downloadDataAudit_onFinishedOK(fullFilePathAndName, fileID);
			
			//notifico
			if (NULL != subscriber)
                notify_READ_DATA_AUDIT_PROGRESS (*subscriber, handlerID, logger, eReadDataFileStatus_finishedOK, kbReadSoFar, fileID);

            return eReadDataFileStatus_finishedOK;
        }

        //notifico che ho letto un altro Kb
        if (kbReadSoFar != lastKbReadSent)
        {
            lastKbReadSent = kbReadSoFar;

            if (NULL!= subscriber)
                notify_READ_DATA_AUDIT_PROGRESS (*subscriber, 0, logger, eReadDataFileStatus_inProgress, kbReadSoFar, fileID);
        }
    }
}

//***************************************************
void Server::priv_downloadDataAudit_onFinishedOK(const char *fullFilePathAndName, u32 fileID)
{
	//parso il file ricevuto e genero un secondo file di nome "packedDataAudit%d.dat" contenente le info in versione "packed"
	EVADTSParser *parser = RHEANEW(localAllocator, EVADTSParser)();
	if (parser->loadAndParse(fullFilePathAndName))
	{
		priv_retreiveSomeDataFromLocalDA3();

		rhea::Allocator *allocator = rhea::memory_getScrapAllocator();
		u32 bufferSize = 0;
		u8 *buffer = parser->createBufferWithPackedData(allocator, &bufferSize, this->cpu_numDecimalsForPrices);
		if (NULL != buffer)
		{
			char s[256];
			sprintf_s(s, sizeof(s), "%s/temp/packedDataAudit%d.dat", rhea::getPhysicalPathToAppFolder(), fileID);
			FILE *f2 = fopen(s, "wb");
			fwrite(buffer, bufferSize, 1, f2);
			fclose(f2);
			RHEAFREE(allocator, buffer);
		}
	}
	RHEADELETE(localAllocator, parser);
}

//***************************************************
bool Server::priv_askVMCDataFileTimeStampAndWaitAnswer(sCPUVMCDataFileTimeStamp *out, u32 timeoutMSec)
{
	u8 bufferW[16];
	const u8 nBytesToSend = buildMsg_getVMCDataFileTimeStamp(bufferW, sizeof(bufferW));

	//invio richiesta a CPU
	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, timeoutMSec))
	{
		//errore, la CPU non ha risposto, abortisco l'operazione
		return false;
	}

	//la CPU risponde con [#] [T] [len] [secondi] [minuti] [ore] [giorno] [mese] [anno] [ck]
	out->readFromBuffer(&answerBuffer[3]);
	return true;
}


/***************************************************
 * Manda il maessaggio B alla CPU e attende la sua risposta.
 *
 * Ritorna 0 se la CPU non ha risposta
 * Ritorna il numero di byte ricevuti se la CPU ha effettivamente risposto. La risposta viene messa nella variabile di classe "answerBuffer"
 */
u16 Server::priv_prepareAndSendMsg_checkStatus_B (u8 btnNumberToSend)
{
    //se devo mandare un bottone in particolare (per es a seguito di una richiesta CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM,
    //lo faccio qui ma solo quando btnNumberToSend==0.
    //L'idea è che qui sto preparando il solito messaggio di stato...se non avevo nessun btn in particolare da invia (btnNumberToSend==0) e se
    //ne avevo uno in canna in attesa (nextButtonNumToSend!=0), allora lo mando ora
    if (0 == btnNumberToSend)
        btnNumberToSend = keepOnSendingThisButtonNum;

    u8 bufferW[32];
    u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B (btnNumberToSend, lang_getErrorCode(&language), bufferW, sizeof(bufferW));
    u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
    if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 500))
        return 0;
    return sizeOfAnswerBuffer;
}

//***************************************************
void Server::priv_resetInternalState(cpubridge::eVMCState s)
{
	memset(&cpuParamIniziali, 0, sizeof(sCPUParamIniziali));
	memset(&cpuStatus, 0, sizeof(sCPUStatus));
	memset(utf16_lastCPUMsg, 0, sizeof(utf16_lastCPUMsg));
	lastCPUMsg_len = 0;
	lastBtnProgStatus = 0;
	keepOnSendingThisButtonNum = 0;

	cpuStatus.statoPreparazioneBevanda = eStatoPreparazioneBevanda_doing_nothing;
	cpuStatus.VMCstate = s;
}

/***************************************************
 * priv_enterState_compatibilityCheck
 *
 *	Ci si entra all'inizio, quando la connessione con la CPU è stata stabilita.
 *	Serve per verificare che il FW di CPU sia quello giusto
 *	In caso di errore, va nello stato eVMCState_CPU_NOT_COMPATIBLE dal quale non si esce più
 *  In caso di successo, si va nello stato DA3SYNC
 */
void Server::priv_enterState_compatibilityCheck()
{
//priv_enterState_comError(); return;
	logger->log("CPUBridgeServer::priv_enterState_compatibilityCheck()\n");

	stato.set(sStato::eStato_compatibilityCheck);
	priv_resetInternalState(eVMCState_COMPATIBILITY_CHECK);

	cpuStatus.LCDMsg.utf16LCDString[0] = 0x00;
	rhea::utf16::concatFromASCII(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), "COMPATIBILITY    CHECK");
	cpuStatus.LCDMsg.importanceLevel = 123;

	//segnalo ai miei subscriber lo stato corrente
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
	{
		notify_CPU_STATE_CHANGED(subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
		notify_CPU_SEL_AVAIL_CHANGED(subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
		notify_CPU_NEW_LCD_MESSAGE(subscriberList(i)->q, 0, logger, &cpuStatus.LCDMsg);
	}
}

//***************************************************
void Server::priv_handleState_compatibilityCheck()
{
	//preparo il msg da mandare alla CPU
	u8 bufferW[64];
	

	//mando il comando "C" una decina di volte per vedere se la CPU esiste
	u8 nRetry = 10;
	while (1)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		//invio comando initalParam
		const u8 nBytesToSend = cpubridge::buildMsg_initialParam_C(2, 0, 0, bufferW, sizeof(bufferW));
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 200))
		{
			priv_parseAnswer_initialParam(answerBuffer, sizeOfAnswerBuffer);
			if (strcmp(cpuParamIniziali.CPU_version, "FAKE CPU") == 0)
			{
                sCPUVMCDataFileTimeStamp myTS;
                myTS.setInvalid();
                saveVMCDataFileTimeStamp(myTS);
                priv_handleState_DA3Sync();
				return;
			}
			break;
		}

		nRetry--;
		if (nRetry == 0)
		{
            //la CPU non ha mai risposto, ne deduco che non esiste oppure non è in grado di rispondere.
			//Essendo questa la procedera di "boot", vado nello stato NOT_SUPPORTED perchè non sono stato
			//in grado di contattare la CPU nemmeno una volta
			priv_enterState_CPUNotSupported();
			return;
		}

		priv_handleMsgQueues(timeNowMSec, 1000);
	}



    //la CPU ha risposto, a questo punto verifico che supporti il comando "c" che è tipico delle CPU fusion2
	priv_handleMsgQueues(rhea::getTimeNowMSec(), 10);
	nRetry = 4;
	while (1)
	{
		sExtendedCPUInfo info;
		if (priv_prepareSendMsgAndParseAnswer_getExtendedCOnfgInfo_c(&info))
		{
			break;
		}

		nRetry--;
		if (nRetry == 0)
		{
			//La CPU non ha risposto a "c", quindi ne deduco che è di una versione precedente alla 2.0... non va bene
			priv_enterState_CPUNotSupported();
			return;
		}

		priv_handleMsgQueues(rhea::getTimeNowMSec(), 1000);
	}

	//tutto ok, passo a da3Sync
    priv_handleMsgQueues(rhea::getTimeNowMSec(), 1);
	priv_enterState_DA3Sync();
}

/***************************************************
 * Questo stato è una sorta di stato invalido e ci si entra quando, durante le fasi iniziali, la GPU non è riuscita a contattare la CPU
 * oppure la CPU contattata non è una versione valida per questo sw.
 * Da questo stato non si esce mai. E' consentito aggiornare il FW di CPU, ma alla fine è comunque necessario fare il reboot
 */
void Server::priv_enterState_CPUNotSupported()
{
	logger->log("CPUBridgeServer::priv_enterState_CPUNotSupported()\n");

	stato.set(sStato::eStato_CPUNotSupported);
	priv_resetInternalState(eVMCState_CPU_NOT_SUPPORTED);

	cpuStatus.LCDMsg.utf16LCDString[0] = 0;
	rhea::utf16::concatFromASCII(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), "CPU NOT SUPPORTED");
	cpuStatus.LCDMsg.importanceLevel = 123;

	//segnalo ai miei subscriber lo stato corrente
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
	{
		notify_CPU_STATE_CHANGED(subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
		notify_CPU_SEL_AVAIL_CHANGED(subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
		notify_CPU_NEW_LCD_MESSAGE(subscriberList(i)->q, 0, logger, &cpuStatus.LCDMsg);
	}
}

//***************************************************
void Server::priv_handleState_CPUNotSupported()
{
	while (1)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();
		priv_handleMsgQueues(timeNowMSec, 5000);
	}
}

/***************************************************
 * In questo stato la GPU consulta la CPU per verificare lo stato di sincronia del DA3.
 * Se la CPU riporta un timestamp != da quello della GPU, allora automaticamente parte la procedura di download del DA3 e aggiornamento del timestamp.
 * Lo scopo ultimo di questo stato è fare in modo che il DA3 di GPU sia uguale a quello di CPU.
 * Da questo stato si passa automaticamente nello stato "normal" oppure in "com error"
 */
void Server::priv_enterState_DA3Sync()
{
	logger->log("CPUBridgeServer::priv_enterState_DA3Sync()\n");

	stato.set(sStato::eStato_DA3_sync);

	cpuStatus.statoPreparazioneBevanda = eStatoPreparazioneBevanda_doing_nothing;
	cpuStatus.VMCstate = cpubridge::eVMCState_DA3_SYNC;

	//segnalo ai miei subscriber lo stato corrente
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
		notify_CPU_STATE_CHANGED(subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
}

//***************************************************
void Server::priv_handleState_DA3Sync()
{
	//carico il mio TS
	sCPUVMCDataFileTimeStamp myTS;
	loadVMCDataFileTimeStamp(&myTS);

	//se il file da3 non esiste, invalido il mio ts
	char s[256];
	sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
	if (!rhea::fs::fileExists(s))
		myTS.setInvalid();

    priv_handleMsgQueues(rhea::getTimeNowMSec(), 1);

	//chiedo il timestamp alla CPU
	sCPUVMCDataFileTimeStamp cpuTS;
	u8 nRetry = 5;
	while (1)
	{
		if (priv_askVMCDataFileTimeStampAndWaitAnswer(&cpuTS, 100))
		{
			//se il suo TS è == al mio, ho finito
			if (myTS.isEqual(cpuTS))
			{
				priv_retreiveSomeDataFromLocalDA3();
				priv_enterState_normal();
				return;
			}
			break;
		}

		nRetry--;
		if (nRetry == 0)
		{
			priv_enterState_comError();
			return;
		}

		priv_handleMsgQueues(rhea::getTimeNowMSec(), 100);
	}
	

	//se arrivo qui vuol dire che il mio da3 non è in sync con quello della CPU, procedo al download
    priv_handleMsgQueues(rhea::getTimeNowMSec(), 1);
	nRetry = 2;
	while (nRetry--)
	{
		u16 fileID = 0;
		if (eReadDataFileStatus_finishedOK == priv_downloadVMCDataFile(NULL, 0, &fileID))
		{
			//tutto ok, il file è stato scaricato in temp/vmcDataFile%d.da3
			//Lo copio in current/da3, aggiorno il timestamp, aggiorno la data di ultima modifica

			char s1[256];
			sprintf_s(s, sizeof(s), "%s/temp/vmcDataFile%d.da3", rhea::getPhysicalPathToAppFolder(), fileID);
			sprintf_s(s1, sizeof(s1), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
			rhea::fs::fileDelete(s1);
			rhea::fs::fileCopy(s, s1);

			//aggiorno il ts
			saveVMCDataFileTimeStamp(cpuTS);


			//Se nella cartella last_installed non c'è nulla, allora il da3 scaricato dalla CPU
			//lo chiamo "downloaded_from_cpu.da3", altrimenti mantengo il nome del file precedente
			char dstFilename[128];
			sprintf_s(dstFilename, sizeof(dstFilename), "download_from_cpu.da3");

			sprintf_s(s1, sizeof(s1), "%s/last_installed/da3", rhea::getPhysicalPathToAppFolder());
			OSFileFind ff;
			if (rhea::fs::findFirst(&ff, s1, "*.da3"))
			{
				do
				{
					if (!rhea::fs::findIsDirectory(ff))
					{
						sprintf_s(dstFilename, sizeof(dstFilename), "%s", rhea::fs::findGetFileName(ff));
						break;
					}
				} while (rhea::fs::findNext(ff));
				rhea::fs::findClose(ff);
			}
			rhea::fs::deleteAllFileInFolderRecursively(s1, false);

			//copio anche in cartella last_installed
			sprintf_s(s1, sizeof(s1), "%s/last_installed/da3/%s", rhea::getPhysicalPathToAppFolder(), dstFilename);
			rhea::fs::fileCopy(s, s1);

			//aggiorno il file con data e ora di ultima modifica
			rhea::DateTime dt;
			dt.setNow();
			sprintf_s(s, sizeof(s), "%s/last_installed/da3/dateUM.bin", rhea::getPhysicalPathToAppFolder());
			FILE *f = fopen(s, "wb");
			u64 u = dt.getInternalRappresentation();
			fwrite(&u, sizeof(u64), 1, f);
			fclose(f);

			//finito
			priv_retreiveSomeDataFromLocalDA3();
			priv_enterState_normal();
			return;
		}
	}

	priv_enterState_comError();
}




/***************************************************
 * Se la CPU ha smesso di rispondere, si entra in questo stato.
 * Una volta qui dentro, la GPU comincerà a mandare il comando "C" perdiodicamente (circa una volta ogni 3 secondi).
 * Se la CPU risponde, si passa automaticamente allo stato DA3Sync.
 * Fintanto che la CPU non risponde, si rimane qui dentro all'infinito, inviando periodicamente il comando "C" per vedere se la CPU è viva
 */
void Server::priv_enterState_comError()
{
	logger->log("CPUBridgeServer::priv_enterState_comError()\n");

    stato.set (sStato::eStato_comError);
	priv_resetInternalState(eVMCState_COM_ERROR);

	cpuStatus.LCDMsg.utf16LCDString[0] = 0;
	rhea::utf16::concatFromASCII(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), "COM ERROR");
    cpuStatus.LCDMsg.importanceLevel = 123;

    //segnalo ai miei subscriber che sono in com-error
    for (u32 i = 0; i < subscriberList.getNElem(); i++)
    {
        notify_CPU_STATE_CHANGED (subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
        notify_CPU_SEL_AVAIL_CHANGED (subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
        notify_CPU_NEW_LCD_MESSAGE (subscriberList(i)->q, 0, logger, &cpuStatus.LCDMsg);
    }
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
    while (stato.get() == sStato::eStato_comError)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		//invio comando initalParam
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
		{
			//la CPU ha risposto, elaboro la risposta e passo in stato "normal"
			priv_parseAnswer_initialParam (answerBuffer, sizeOfAnswerBuffer);
			//priv_enterState_normal();
			priv_enterState_DA3Sync();
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
}


/***************************************************
 * Questo è il normale stato operativo. La CPU è stata contattata, è viva, ha riportato una versione supportata e il file DA3 è stato
 * sincronizzato.
 */
void Server::priv_enterState_normal()
{
	logger->log("CPUBridgeServer::priv_enterState_normal()\n");
    stato.set (sStato::eStato_normal);
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
	const u8 ALLOW_N_RETRY_BEFORE_COMERROR = 5;
	u8 nRetry = 0;

	u64	nextTimeSendCheckStatusMsgWasMSec = 0;
    while (stato.get() == sStato::eStato_normal)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		//ogni tot, invio un msg di stato alla CPU
		if (timeNowMSec >= nextTimeSendCheckStatusMsgWasMSec)
		{
            u16 sizeOfAnswerBuffer = priv_prepareAndSendMsg_checkStatus_B(0);
            if (0 == sizeOfAnswerBuffer)
			{
				//la CPU non ha risposto al mio comando di stato, passo in com_error
				nRetry++;
				if (nRetry >= ALLOW_N_RETRY_BEFORE_COMERROR)
				{
					priv_enterState_comError();
					return;
				}
			}
			else
			{
				//parso la risposta
				nRetry = 0;
				priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);

                if (this->cpuStatus.VMCstate == eVMCState_PROGRAMMAZIONE)
                {
                    priv_enterState_programmazione();
                    return;
                }

				// se la CPU mi sta dicendo che è in telemetria...
				if ((cpuStatus.flag1 & sCPUStatus::FLAG1_TELEMETRY_RUNNING) != 0)
				{
					priv_enterState_telemetry();
					return;
				}
			}

			//schedulo il prossimo msg di stato
			nextTimeSendCheckStatusMsgWasMSec = rhea::getTimeNowMSec() + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;
		}

		//ci sono messaggi in ingresso?
		priv_handleMsgQueues (timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
	}
}


/***************************************************
 * In questo stato si entra quando la CPU va in stato PROGRAMMAZIONE.
 *	Da questo stato si esce quando la CPU esce dallo stato PROGRAMMAZIONE.
 *	All'uscita si va in DA3_SYNC
 */
void Server::priv_enterState_programmazione()
{
    logger->log("CPUBridgeServer::priv_enterState_programmazione()\n");
    stato.set (sStato::eStato_programmazione);
}

//***************************************************
void Server::priv_handleState_programmazione()
{
    const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 50;
    const u8 ALLOW_N_RETRY_BEFORE_COMERROR = 8;
    u8 nRetry = 0;

    u64	nextTimeSendCheckStatusMsgWasMSec = 0;
    while (stato.get() == sStato::eStato_programmazione)
    {
        const u64 timeNowMSec = rhea::getTimeNowMSec();

        //ogni tot, invio un msg di stato alla CPU
        if (timeNowMSec >= nextTimeSendCheckStatusMsgWasMSec)
        {
            u16 sizeOfAnswerBuffer = priv_prepareAndSendMsg_checkStatus_B(0);
            if (0 == sizeOfAnswerBuffer)
            {
                //la CPU non ha risposto al mio comando di stato, passo in com_error
                nRetry++;
                if (nRetry >= ALLOW_N_RETRY_BEFORE_COMERROR)
                {
                    priv_enterState_comError();
                    return;
                }
            }
            else
            {
                //parso la risposta
                nRetry = 0;
                priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);

                if (this->cpuStatus.VMCstate != eVMCState_PROGRAMMAZIONE)
                {
                    //priv_enterState_normal();
                    priv_enterState_DA3Sync();
                    return;
                }
            }

            //schedulo il prossimo msg di stato
            nextTimeSendCheckStatusMsgWasMSec = rhea::getTimeNowMSec() + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;
        }

        //ci sono messaggi in ingresso?
        priv_handleMsgQueues (timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
    }
}


/***************************************************
 * In questo stato si entra quando la CPU tira su un preciso bit durante la risposta al comando
 *	'B'. Una volta entrati in questo stato, ci aspettiamo che la CPU possa non rispondere ad alcun
 *	comando per parecchio tempo. Durante questo stato, ci comportiamo come se fossimo in com_error solo
 *	che il messaggio visualizzato a video è "telemetry in progress" invece che "com error".
 *	Usiamo da qui quando la CPU ricomincia a rispondere e tira giu il bit di cui sopra
 */
void Server::priv_enterState_telemetry()
{
	logger->log("CPUBridgeServer::priv_enterState_telemetry()\n");
	stato.set(sStato::eStato_telemetry);
}

//***************************************************
void Server::priv_handleState_telemetry()
{
	const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 500;

	u64	nextTimeSendCheckStatusMsgWasMSec = 0;
	while (stato.get() == sStato::eStato_telemetry)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		//ogni tot, invio un msg di stato alla CPU
		if (timeNowMSec >= nextTimeSendCheckStatusMsgWasMSec)
		{
			u16 sizeOfAnswerBuffer = priv_prepareAndSendMsg_checkStatus_B(0);
			if (0 != sizeOfAnswerBuffer)
			{
				//parso la risposta
				priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);

				// se non siamo più in telemetria, esco da questo stato
				if ((cpuStatus.flag1 & sCPUStatus::FLAG1_TELEMETRY_RUNNING) == 0)
				{
					priv_enterState_DA3Sync();
					return;
				}
			}

			//schedulo il prossimo msg di stato
			nextTimeSendCheckStatusMsgWasMSec = rhea::getTimeNowMSec() + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;
		}

		//ci sono messaggi in ingresso?
		priv_handleMsgQueues(timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
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
void Server::priv_parseAnswer_checkStatus (const u8 *answer, u16 answerLen UNUSED_PARAM)
{
	u8 isMultilangage = 0;
	const u8 prevMsgLcdCPUImportanceLevel = cpuStatus.LCDMsg.importanceLevel;
	cpuStatus.LCDMsg.importanceLevel = 0xff;
	u16 newCpuStatusFlag1 = cpuStatus.flag1;

	if (answer[1] != eCPUCommand_checkStatus_B && answer[1] != eCPUCommand_checkStatus_B_Unicode)
	{
		//mi aspettavo la risposta al mio comando B, invece ho ricevuto dell'altro
		logger->log("WARN: I was expecintg B or Z, received [%c]\n", (char)answer[1]);
		return;
	}

	if (cpuParamIniziali.protocol_version >= 1)
	{
		u8 z = 17 + 32;
		if (answer[1] != eCPUCommand_checkStatus_B)
			z += 32;
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
					lang_clearErrorCode(&language);
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
					const char *curLang = lang_getCurLanguage (&language);
					if (curLang[0] != language_requested[0] || curLang[1] != language_requested[1])
					{
						language_requested[2] = 0x00;
						lang_open (&language, language_requested);
					}
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


				if (cpuParamIniziali.protocol_version >= 4)
				{
					//1 byte per indicare se il btnProg è attualmente in stato di PRESSED
					u8 btnProgIsPressedNOW = answer[z++];
					if (btnProgIsPressedNOW == 1)
					{
						if (lastBtnProgStatus == 0)
						{
							//la CPU mi segnala che è stato premuto il btnPROG, diramo l'evento a tutti
							lastBtnProgStatus = 1;
							for (u32 i = 0; i < subscriberList.getNElem(); i++)
								notify_CPU_BTN_PROG_PRESSED(subscriberList(i)->q, 0, logger);
						}
					}
					else
					{
						lastBtnProgStatus = 0;
					}					

					if (cpuParamIniziali.protocol_version >= 5)
					{
						//1 byte con 8 bit a mo di flag per usi futuri
						const u8 flag = answer[z++];

						//bit 0x01 == 1 quando la CPU è pronta per scaricare eva-dts
						if ((flag & 0x01) != 0)
							newCpuStatusFlag1 |= sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT;
						else
							newCpuStatusFlag1 &= (~sCPUStatus::FLAG1_READY_TO_DELIVER_DATA_AUDIT);

						if (cpuParamIniziali.protocol_version >= 6)
						{
							if ((flag & 0x02) != 0)
								newCpuStatusFlag1 |= sCPUStatus::FLAG1_TELEMETRY_RUNNING;
							else
								newCpuStatusFlag1 &= (~sCPUStatus::FLAG1_TELEMETRY_RUNNING);
							
						}//if (cpuParamIniziali.protocol_version >= 6)						
					}//if (cpuParamIniziali.protocol_version >= 5)
				}//if (cpuParamIniziali.protocol_version >= 4)
			}//if (cpuParamIniziali.protocol_version >= 3)
		} // if (cpuParamIniziali.protocol_version >= 2)
	} //if (cpuParamIniziali.protocol_version >= 1)


	
	//cpuStatus.CupAbsentStatus_flag = answer[9] & 0x08;
	if ((answer[9] & 0x08) != 0)
		newCpuStatusFlag1 |= sCPUStatus::FLAG1_CUP_ABSENT;
	else
		newCpuStatusFlag1 &= (~sCPUStatus::FLAG1_CUP_ABSENT);

	//cpuStatus.bShowDialogStopSelezione = answer[9] & 0x10;
	if ((answer[9] & 0x10) != 0)
		newCpuStatusFlag1 |= sCPUStatus::FLAG1_SHOW_DLG_STOP_SELEZIONE;
	else
		newCpuStatusFlag1 &= (~sCPUStatus::FLAG1_SHOW_DLG_STOP_SELEZIONE);



	//stato della CPU
	u8 bTriggerEvent = 0;
	if (cpuStatus.VMCstate != (eVMCState)answer[3])		{ cpuStatus.VMCstate = (eVMCState)answer[3]; bTriggerEvent = 1; }
	if (cpuStatus.VMCerrorCode != answer[4])			{ cpuStatus.VMCerrorCode = answer[4]; bTriggerEvent = 1; }
	if (cpuStatus.VMCerrorType != answer[5])			{ cpuStatus.VMCerrorType = answer[5]; bTriggerEvent = 1; }
	if (bTriggerEvent || newCpuStatusFlag1 != cpuStatus.flag1)
	{
		cpuStatus.flag1 = newCpuStatusFlag1;
		for (u32 i = 0; i < subscriberList.getNElem(); i++)
			notify_CPU_STATE_CHANGED (subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
	}

	
	

	

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

    //u8 selection_CPU_current = answer[10];


	//la CPU invia un messaggio testuale, può essere in ASCII o in unicode. Io lo traduco sempre in utf16
	//A seconda che la risposta CPU sia in ASCII o UNICODE, ci sono quindi 32 o 64 bytes di messaggio
	u16	utf16_msgLCD[sCPULCDMessage::BUFFER_SIZE_IN_U16];
	u8  msgLCD_len = 0;
	memset(utf16_msgLCD, 0, sizeof(utf16_msgLCD));

	u8 z = 11;
	u16 firstGoodChar = ' ';
	for (u8 i = 0; i < 32; i++)
	{
		if (answer[1] == 'B')
			utf16_msgLCD[msgLCD_len] = answer[z++];
		else //answer[1] == 'Z'
		{
			utf16_msgLCD[msgLCD_len] = (u16)answer[z] + (u16)answer[z + 1] * 256;
			z += 2;
		}

		if (utf16_msgLCD[msgLCD_len] != ' ' && firstGoodChar == ' ')
			firstGoodChar = utf16_msgLCD[msgLCD_len];
		msgLCD_len++;

		//mette uno spazio dopo i primi 16 caratteri perchè storicamente il msg di CPU è composto da 2 messaggi da 16 char da visualizzare
		//uno sotto l'altro. Noi invece li visualizziamo sulla stessa riga
		if (msgLCD_len == 16)
		{
			if (!isMultilangage || firstGoodChar != '@')
				utf16_msgLCD[msgLCD_len++] = ' ';
		}
	}
	assert(msgLCD_len <= sCPULCDMessage::BUFFER_SIZE_IN_U16);
	utf16_msgLCD[msgLCD_len] = 0;
	rhea::utf16::rtrim(utf16_msgLCD);

	//se la CPU ha alzato il bit di "telemetria in corso", devo fare un override del messaggio testuale
	if ((cpuStatus.flag1 & sCPUStatus::FLAG1_TELEMETRY_RUNNING) != 0)
	{
		utf16_msgLCD[0] = 0;
		rhea::utf16::concatFromASCII (utf16_msgLCD, sizeof(utf16_msgLCD), "TELEMETRY RUNNING...");
		msgLCD_len = rhea::utf16::length(utf16_msgLCD);
	}


	
	//1 bit per ogni selezione per indicare se la selezione è disponibile o no
	//Considerando che NumMaxSelections=48, dovrebbero servire 6 byte
	//ATTENZIONE che bit==0 significa che la selezione è OK, bit==1 significa KO
	//Io invece traduco al contrario, per cui per me cupStatus.selAvailability == 1 se la selezione è disponibile
	u8 anythingChanged = 0;


	//se la CPU ha alzato il bit di "telemetria in corso", devo disabilitare d'ufficio tutte le selezioni
	if ((cpuStatus.flag1 & sCPUStatus::FLAG1_TELEMETRY_RUNNING) != 0)
	{
		if (cpuStatus.selAvailability.areAllNotAvail() == false)
		{
			cpuStatus.selAvailability.reset();
			anythingChanged = 1;
		}
	}
	else
	{
		if (cpuStatus.VMCstate == eVMCState_DISPONIBILE || cpuStatus.VMCstate == eVMCState_PREPARAZIONE_BEVANDA)
		{
			u8 mask = 0x01;
			for (u8 i = 0; i < NUM_MAX_SELECTIONS; i++)
			{
				u8 isSelectionAvail = 1;
				if ((answer[z] & mask) != 0)
					isSelectionAvail = 0;

				if (isSelectionAvail)
				{
					if (!cpuStatus.selAvailability.isAvail(i + 1))
					{
						anythingChanged = 1;
						cpuStatus.selAvailability.setAsAvail(i + 1);
					}
				}
				else
				{
					if (cpuStatus.selAvailability.isAvail(i + 1))
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
		}
		else
		{
			//la CPU non è in uno stato valido per fare erogazioni, per cui forzo a priori  la totalte
			//indisponibilità delle bevande
			if (cpuStatus.selAvailability.areAllNotAvail() == false)
			{
				cpuStatus.selAvailability.reset();
				anythingChanged = 1;
			}
		}
	}
	if (anythingChanged)
	{
		for (u32 i = 0; i < subscriberList.getNElem(); i++)
			notify_CPU_SEL_AVAIL_CHANGED (subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
	}

	//se non c'è nemmeno una selezione disponibile, mostro sempre e cmq il msg di CPU anche se non fosse "importante"
	if (cpuStatus.selAvailability.areAllNotAvail())
		cpuStatus.LCDMsg.importanceLevel = 0xff;


	//se il messaggio LCD è cambiato dal giro precedente, oppure lo stato di importanza è cambiato, devo notificare il nuovo messaggio a tutti
	bool bDoNotifyNewLCDMessage = false;
    if (prevMsgLcdCPUImportanceLevel != cpuStatus.LCDMsg.importanceLevel || msgLCD_len != lastCPUMsg_len || memcmp(utf16_msgLCD, utf16_lastCPUMsg, lastCPUMsg_len*2 + 1) != 0)
	{
		memcpy(utf16_lastCPUMsg, utf16_msgLCD, (msgLCD_len + 1) * 2);
		lastCPUMsg_len = msgLCD_len;
		memcpy(cpuStatus.LCDMsg.utf16LCDString, utf16_msgLCD, (msgLCD_len + 1) * 2);

		//lo traduco se necessario
		if (isMultilangage)
		{
			//se il primo ch diverso da "spazio" è "@", allora stiamo parlano di un messaggio custom
			u16 i = 0;
			while (utf16_lastCPUMsg[i] != 0x00)
			{
				if (utf16_lastCPUMsg[i] != ' ')
					break;
				++i;
			}
			if (utf16_lastCPUMsg[i] == LANG_CHIOCCIOLA)
			{
				u16 t = 0;
				while (utf16_lastCPUMsg[i] != 0x00)
					cpuStatus.LCDMsg.utf16LCDString[t++] = utf16_lastCPUMsg[i++];
				cpuStatus.LCDMsg.utf16LCDString[t] = 0x00;
				t = rhea::utf16::rtrim(cpuStatus.LCDMsg.utf16LCDString);

				if (t > 0)
					t = lang_translate(&language, cpuStatus.LCDMsg.utf16LCDString, sCPULCDMessage::BUFFER_SIZE_IN_U16 - 1);
			}
		}

		bDoNotifyNewLCDMessage = true;
	}


    //Se sono nella modalità "mostra la stringa con cpu model and version", per tot secondi prependo il nome modello all'attuale msg di CPU
	/*showCPUStringModelAndVersionUntil_msec = 10;
	utf16_CPUMasterVersionString[0] = 0;
	rhea::utf16::concatFromASCII(utf16_CPUMasterVersionString, sizeof(utf16_CPUMasterVersionString), "xxxxxxxxxTS30 02.03.00    GB-GB");
	*/
	if (showCPUStringModelAndVersionUntil_msec)
    {
		const u16 utf16_spacer[] = { 0x0020, 0x002d, 0x0020, 0x0000 }; // è la stringa " - "
		if (bDoNotifyNewLCDMessage)
		{
			//e' arrivato un msg nuovo dalla CPU, devo prependere la stringa con il modello
			if (cpuStatus.LCDMsg.utf16LCDString[0] != 0x0000)
				rhea::utf16::prepend(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), utf16_spacer);
			rhea::utf16::prepend(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), utf16_CPUMasterVersionString);
            lastCPUMsg_len = rhea::utf16::length (cpuStatus.LCDMsg.utf16LCDString);
            cpuStatus.LCDMsg.importanceLevel = 0xff;
		}
		else
		{
			//rispetto all'ultima volta, CPU non ha mandato nuovi messaggi.
			//A questo punto, se cpuStatus.LCDMsg.utf16LCDString inizia già con la stringa col modello, non devo fare nulla, altrimenti la devo prependere
			//e notificare tutti
			u32 n = rhea::utf16::length(utf16_CPUMasterVersionString);
			if (memcmp(cpuStatus.LCDMsg.utf16LCDString, utf16_CPUMasterVersionString, n * 2) != 0)
			{
				if (cpuStatus.LCDMsg.utf16LCDString[0] != 0x0000)
					rhea::utf16::prepend(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), utf16_spacer);
				rhea::utf16::prepend(cpuStatus.LCDMsg.utf16LCDString, sizeof(cpuStatus.LCDMsg.utf16LCDString), utf16_CPUMasterVersionString);
				bDoNotifyNewLCDMessage = true;
				cpuStatus.LCDMsg.importanceLevel = 0xff;
                lastCPUMsg_len = rhea::utf16::length (cpuStatus.LCDMsg.utf16LCDString);
			}
		}
		
        if (rhea::getTimeNowMSec() >= showCPUStringModelAndVersionUntil_msec)
            showCPUStringModelAndVersionUntil_msec = 0;
    }




	//se richiesto, notifico tutti del nuovo messaggio LCD di CPU
	if (bDoNotifyNewLCDMessage)
	{
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

    if (stato.get() != sStato::eStato_normal)
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


    stato.set (sStato::eStato_selection);
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
    const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 500;
	
	//la cpu deve passare da DISPONIBILE a "BEVANDA IN PREPARAZIONE" entro il tempo definito qui sotto
	const u32 TIMEOUT_SELEZIONE_1_MSEC = 60000;

	//una volta che la CPU è entrata in "PREPARAZIONE", deve tornare disponibile entro il tempo definito qui sotto
	const u32 TIMEOUT_SELEZIONE_2_MSEC = 240000;

    const u8 ALLOW_N_RETRY_BEFORE_COMERROR = 6;
	u8 nRetry = 0;

	assert (runningSel.selNum >= 1 && runningSel.selNum <= NUM_MAX_SELECTIONS);


	u64	timeStartedMSec = rhea::getTimeNowMSec();
	u8 bBevandaInPreparazione = 0;

	//loop fino alla fine della selezione
	u64	nextTimeSendCheckStatusMsgMSec = 0;
    while (stato.get() == sStato::eStato_selection)
	{
		u64 timeNowMSec = rhea::getTimeNowMSec();

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
        u16 sizeOfAnswerBuffer = priv_prepareAndSendMsg_checkStatus_B(selNumberToSend);
        if (0 == sizeOfAnswerBuffer)
		{
			//la CPU non ha risposto al mio comando di stato, passo in com_error
			nRetry++;
			if (nRetry >= ALLOW_N_RETRY_BEFORE_COMERROR)
			{
				runningSel.status = eRunningSelStatus_finished_KO;
				if (runningSel.sub)
					notify_CPU_RUNNING_SEL_STATUS(runningSel.sub->q, 0, logger, runningSel.status);
				priv_enterState_comError();
				return;
			}
		}
		else
		{
			//parso la risposta
			priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);
			nRetry = 0;
		}

		//schedulo il prossimo msg di stato
		timeNowMSec = rhea::getTimeNowMSec();
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

/***************************************************
 * priv_enterState_regolazioneAperturaMacina
 *
 *	ritorna true se ci sono le condizioni per iniziare una regolazione della macina. In questo caso, lo stato passa a stato = eStato_regolazioneAperturaMacina.
 *	In caso contrario, ritorna false e non cambia l'attuale stato.
 */
bool Server::priv_enterState_regolazioneAperturaMacina (u8 macina_1o2, u16 target)
{
	logger->log("CPUBridgeServer::priv_enterState_regolazioneAperturaMacina() => [%d] [%d]\n", macina_1o2, target);

	if (stato.get() != sStato::eStato_normal && (stato.get() != sStato::eStato_regolazioneAperturaMacina))
	{
		logger->log("  invalid request, CPUServer != eStato_normal, aborting.");
		return false;
	}

	if (cpuStatus.VMCstate != eVMCState_DISPONIBILE)
	{
		logger->log("  invalid request, VMCState != eVMCState_DISPONIBILE, aborting.");
		return false;
	}


	stato.set(sStato::eStato_regolazioneAperturaMacina);
	regolazioneAperturaMacina.macina_1o2 = macina_1o2;
	regolazioneAperturaMacina.target = target;
	return true;
}

//***************************************************
void Server::priv_handleState_regolazioneAperturaMacina()
{
	static const u8 NRETRY = 5;
	u8 nRetry = NRETRY;

	eCPUProgrammingCommand_macinaMove move = eCPUProgrammingCommand_macinaMove_stop;
	priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, move);


	//dico a tutti che sono in uno stato speciale
	cpuStatus.VMCstate = eVMCState_REG_APERTURA_MACINA;
	cpuStatus.VMCerrorCode = 0;
	cpuStatus.VMCerrorType = 0;
	cpuStatus.selAvailability.reset();
	for (u32 i = 0; i < subscriberList.getNElem(); i++)
	{
		notify_CPU_STATE_CHANGED(subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType, cpuStatus.flag1);
		notify_CPU_SEL_AVAIL_CHANGED(subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
	}


	const u16 TOLLERANZA = 0;
	while (stato.get() == sStato::eStato_regolazioneAperturaMacina)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		//ci sono messaggi in ingresso?
		priv_handleMsgQueues(timeNowMSec, 1);

			   
		//chiede la posizione della macina
		u16 curpos = 0;
		if (priv_sendAndHandleGetPosizioneMacina (regolazioneAperturaMacina.macina_1o2, &curpos))
		{
			nRetry = NRETRY;

			u16 diff = 0;
			if (curpos >= regolazioneAperturaMacina.target)
				diff = curpos - regolazioneAperturaMacina.target;
			else
				diff = regolazioneAperturaMacina.target - curpos;
			if (diff <= TOLLERANZA)
			{
				//fine
				priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
				//priv_enterState_normal();
				break;
			}

			if (regolazioneAperturaMacina.target > curpos)
			{
				if (move != eCPUProgrammingCommand_macinaMove_open)
				{
					if (move != eCPUProgrammingCommand_macinaMove_stop)
						priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
					move = eCPUProgrammingCommand_macinaMove_open;
					priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, move);
				}
			}
			else
			{
				if (move != eCPUProgrammingCommand_macinaMove_close)
				{
					if (move != eCPUProgrammingCommand_macinaMove_stop)
						priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
					move = eCPUProgrammingCommand_macinaMove_close;
					priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, move);
				}
			}
		}
		else
		{
			nRetry--;
			if (nRetry == 0)
			{
				//abortisco
				priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
				priv_enterState_normal();
				return;
			}
		}
	}

	//a questo punto, sono molto vicino al target, generalmente disto 1 o 2 punti dal valore desiderato, vado di fine tuning
	nRetry = NRETRY;
	while (1)
	{
		//chiede la posizione della macina
		u16 curpos = 0;
		u8 n = 0;
		for (u8 i = 0; i < 3; i++)
		{
			priv_handleMsgQueues(rhea::getTimeNowMSec(), 100);
			rhea::thread::sleepMSec(300);

			u16 pos = 0;
			if (priv_sendAndHandleGetPosizioneMacina(regolazioneAperturaMacina.macina_1o2, &pos))
			{
				curpos += pos;
				n++;
			}

			priv_handleMsgQueues(rhea::getTimeNowMSec(), 100);
		}

		if (n > 0)
		{
			curpos /= n;
			nRetry = NRETRY;

			if (curpos == regolazioneAperturaMacina.target)
			{
				//fine
				priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
				priv_enterState_normal();
				return;
			}

			if (curpos > regolazioneAperturaMacina.target)
				priv_sendAndHandleSetMotoreMacina (regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_close);
			else
				priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_open);

			priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
		}
		else
		{
			nRetry--;
			if (nRetry == 0)
			{
				//abortisco
				priv_sendAndHandleSetMotoreMacina(regolazioneAperturaMacina.macina_1o2, eCPUProgrammingCommand_macinaMove_stop);
				priv_enterState_normal();
				return;
			}
		}
	}
}

//**********************************************
bool Server::priv_sendAndHandleGetPosizioneMacina (u8 macina_1o2, u16 *out)
{
	u8 bufferW[16];
	const u16 nBytesToSend = cpubridge::buildMsg_getPosizioneMacina(bufferW, sizeof(bufferW), macina_1o2);
	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 400))
		return false;

	*out = rhea::utils::bufferReadU16_LSB_MSB(&answerBuffer[5]);
	return true;
}

//**********************************************
bool Server::priv_sendAndHandleSetMotoreMacina(u8 macina_1o2, eCPUProgrammingCommand_macinaMove m)
{
	u8 bufferW[16];
	const u16 nBytesToSend = cpubridge::buildMsg_setMotoreMacina(bufferW, sizeof(bufferW), macina_1o2, m);
	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	
	u8 nRetry = 8;
	while (nRetry--)
	{
		if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 400))
		{
			rhea::thread::sleepMSec(100);
			return true;
		}
	}
	return false;
}

//**********************************************
void Server::priv_retreiveSomeDataFromLocalDA3()
{
	char s[256];
	sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
	u32 sizeOfBuffer = 0;
	u8 *da3 = rhea::fs::fileCopyInMemory(s, localAllocator, &sizeOfBuffer);
	if (NULL == da3)
		return;

	//Numero di cifre decimali da utilizzare durante la formattazione dei prezzi. Tale numero lo trovo nel DA3 alla loc 7066
	this->cpu_numDecimalsForPrices = da3[7066];

	RHEAFREE(localAllocator, da3);
}
