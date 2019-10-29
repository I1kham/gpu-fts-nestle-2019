#include "CPUBridgeServer.h"
#include "CPUBridge.h"
#include "CPUChannelCom.h"
#include "CPUBridgeVersion.h"
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
	bQuit = false;
	priv_enterState_comError();

	while (bQuit == false)
	{
        switch (stato.get())
		{
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
void Server::priv_handleMsgQueues(u64 timeNowMSec UNUSED_PARAM, u32 timeOutMSec)
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
            if (stato.get() == sStato::eStato_selection)
				runningSel.stopSelectionWasRequested = 1;
			break;

        case CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM:
            {
                //Ho optato per inviare subito il comando B invece che schedularlo al prossimo giro
                keepOnSendingThisButtonNum=0;
                u8 btnToSend = 0;
                translate_CPU_SEND_BUTTON(msg, &btnToSend);

                u8 bufferW[32];
                const u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B (btnToSend, lang_getErrorCode(&language), bufferW, sizeof(bufferW));
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
                const u16 nBytesToSend = cpubridge::buildMsg_checkStatus_B (keepOnSendingThisButtonNum, lang_getErrorCode(&language), bufferW, sizeof(bufferW));
                u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
                if (chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 500))
                    priv_parseAnswer_checkStatus(answerBuffer, sizeOfAnswerBuffer);
            }
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

        case CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT:
            if (stato.get() == sStato::eStato_normal && stato.whatToDo() == sStato::eWhatToDo_nothing)
                priv_downloadDataAudit(&sub->q, handlerID);
            else
                //rifiuto la richiesta perch� non sono in uno stato valido per la lettura del data audit
                notify_READ_DATA_AUDIT_PROGRESS (sub->q, handlerID, logger, eReadDataFileStatus_finishedKO_cantStart_invalidState, 0, 0);
            break;

		case CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE:
			if (stato.get() == sStato::eStato_normal && stato.whatToDo() == sStato::eWhatToDo_nothing)
				priv_downloadVMCDataFile(&sub->q, handlerID);
			else
				//rifiuto la richiesta perch� non sono in uno stato valido per la lettura del file
				notify_READ_VMCDATAFILE_PROGRESS(sub->q, handlerID, logger, eReadDataFileStatus_finishedKO_cantStart_invalidState, 0, 0);
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE:
			{
				char srcFullFileNameAndPath[512];
				translate_WRITE_VMCDATAFILE(msg, srcFullFileNameAndPath, sizeof(srcFullFileNameAndPath));
				if (stato.get() == sStato::eStato_normal && stato.whatToDo() == sStato::eWhatToDo_nothing)
					priv_uploadVMCDataFile(&sub->q, handlerID, srcFullFileNameAndPath);
				else
					//rifiuto la richiesta perch� non sono in uno stato valido per la scrittura del file
					notify_WRITE_VMCDATAFILE_PROGRESS(sub->q, handlerID, logger, eWriteDataFileStatus_finishedKO_cantStart_invalidState, 0);
			}
			break;

		case CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP:
			{
				sCPUVMCDataFileTimeStamp ts;
				if (!priv_askVMCDataFileTimeStampAndWaitAnswer(&ts))
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

        case CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD:
            {
                eCPUProgrammingCommand c;
				const u8 *optionalData;
                cpubridge::translate_CPU_PROGRAMMING_CMD (msg, &c, &optionalData);
				priv_handleProgrammingMessage(sub, c, optionalData);
            }
            break;

		}
		rhea::thread::deleteMsg(msg);
	}
}


//**********************************************
void Server::priv_handleProgrammingMessage(sSubscription *sub, eCPUProgrammingCommand cmd, const u8 *optionalData)
{
	u8 bufferW[32];
	u8 nBytesToSend = 0;
	
	switch (cmd)
	{
	default:
		logger->log("ERR: invalid prog command [%d]\n", (u8)cmd);
		break;

	case eCPUProgrammingCommand_enterProg:
		nBytesToSend = cpubridge::buildMsg_Programming(cmd, NULL, 0, bufferW, sizeof(bufferW));
		break;

	case eCPUProgrammingCommand_cleaning:
		//in [optionalData] c'� un byte che indica il tipo di lavaggio
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
	}
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
 * Al termine dell'upload, se tutto � andato bene, il mhx sorgente viene copiato pari pari (compreso il suo nome originale) in
 *	app/last_installed/cpu/nomeFileSrc.mhx
 */
eWriteCPUFWFileStatus Server::priv_uploadCPUFW(cpubridge::sSubscriber *subscriber, u16 handlerID, const char *srcFullFileNameAndPath)
{
	char cpuFWFileNameOnly[256];
	char tempFilePathAndName[512];
	rhea::fs::extractFileNameWithExt(srcFullFileNameAndPath, cpuFWFileNameOnly, sizeof(cpuFWFileNameOnly));

	if (strncmp(srcFullFileNameAndPath, "APP:/temp/", 10) == 0)
	{
		//il file si suppone gi� esistere nella mia cartella temp
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


	//per prima cosa si fa fare il reboot alla CPU
	u8 bufferW[CPUFW_BLOCK_SIZE+16];
	u8 nBytesToSend = cpubridge::buildMsg_restart_U(bufferW, 64);
	chToCPU->sendOnlyAndDoNotWait(bufferW, nBytesToSend, logger);

	//aspetto di leggere 'k' dal canale
	if (!chToCPU->waitForASpecificChar('k', 5000))
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
	if (!chToCPU->waitForASpecificChar('M', 3000))
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
 * Al termine dell'upload, se tutto � andatao bene, il da3 sorgente viene copiato pari pari (compreso il suo nome originale) in 
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
		//il file si suppone gi� esistere nella mia cartella temp
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
	//dimensione totale del file. Il tutto � hard-codato...
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
        if (priv_askVMCDataFileTimeStampAndWaitAnswer(&vmcDataFileTimeStamp))
        {
            cpubridge::saveVMCDataFileTimeStamp(vmcDataFileTimeStamp);
            break;
        }
    }


	//notifico il client e finisco
	if (NULL != subscriber)
		notify_WRITE_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eWriteDataFileStatus_finishedOK, kbWrittenSoFar);

	return eWriteDataFileStatus_finishedOK;
}

/***************************************************
 * Chiede alla CPU il file di da3 e lo salva localmente in app/temp/vmcDataFile%d.da3
 * [%d] � un progressivo che viene comunicato alla fine del download nel messaggio di conferma.
 * Periodicamente notifica emettendo un messaggio notify_READ_VMCDATAFILE_PROGRESS() che contiene lo stato di avanzamento
 * del download
 */
eReadDataFileStatus Server::priv_downloadVMCDataFile(cpubridge::sSubscriber *subscriber, u16 handlerID)
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
	//dimensione totale del file. Il tutto � hard-codato...
	u8 bufferW[32];
	u32 nPacketSoFar = 0;
	u32 bytesReadSoFar = 0;
	u16 kbReadSoFar = 0;
	u16 lastKbReadSent = u16MAX;
	while (1)
	{
		u16 nBytesToSend = buildMsg_readVMCDataFile(nPacketSoFar++, bufferW, sizeof(bufferW));
		u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
		if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
		{
			//errore, la CPU non ha risposto, abortisco l'operazione
			if (NULL != subscriber)
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
			if (NULL != subscriber)
				notify_READ_VMCDATAFILE_PROGRESS(*subscriber, handlerID, logger, eReadDataFileStatus_finishedOK, kbReadSoFar, fileID);

			fclose(f);
			return eReadDataFileStatus_finishedOK;
		}

		//notifico che ho letto un altro Kb
		if (kbReadSoFar != lastKbReadSent)
		{
			lastKbReadSent = kbReadSoFar;

			if (NULL != subscriber)
				notify_READ_VMCDATAFILE_PROGRESS(*subscriber, 0, logger, eReadDataFileStatus_inProgress, kbReadSoFar, fileID);
		}
	}
}

/***************************************************
 * Chiede alla CPU il file di data-audit e lo salva localmente in app/temp/dataAudit%d.txt
 * [%d] � un progressivo che viene comunicato alla fine del download nel messaggio di conferma.
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

	FILE *f = fopen(fullFilePathAndName, "wb");
	if (NULL == f)
	{
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
            if (NULL != subscriber)
                notify_READ_DATA_AUDIT_PROGRESS (*subscriber, handlerID, logger, eReadDataFileStatus_finishedOK, kbReadSoFar, fileID);

            fclose(f);
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
bool Server::priv_askVMCDataFileTimeStampAndWaitAnswer(sCPUVMCDataFileTimeStamp *out)
{
	u8 bufferW[16];
	const u8 nBytesToSend = buildMsg_getVMCDataFileTimeStamp(bufferW, sizeof(bufferW));

	//invio richiesta a CPU
	u16 sizeOfAnswerBuffer = sizeof(answerBuffer);
	if (!chToCPU->sendAndWaitAnswer(bufferW, nBytesToSend, answerBuffer, &sizeOfAnswerBuffer, logger, 2000))
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
    //L'idea � che qui sto preparando il solito messaggio di stato...se non avevo nessun btn in particolare da invia (btnNumberToSend==0) e se
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
void Server::priv_enterState_comError()
{
	logger->log("CPUBridgeServer::priv_enterState_comError()\n");

    stato.set (sStato::eStato_comError, sStato::eWhatToDo_nothing);

    memset (&cpuParamIniziali, 0, sizeof(sCPUParamIniziali));
    memset (&cpuStatus, 0, sizeof(sCPUStatus));
    memset(lastCPUMsg, 0, sizeof(lastCPUMsg));
    lastCPUMsgLen = 0;
    lastBtnProgStatus = 0;
    keepOnSendingThisButtonNum = 0;

    cpuStatus.VMCstate = eVMCState_COM_ERROR;
	cpuStatus.statoPreparazioneBevanda = eStatoPreparazioneBevanda_doing_nothing;

    cpuStatus.LCDMsg.buffer[0] = 'C';
    cpuStatus.LCDMsg.buffer[1] = 'O';
    cpuStatus.LCDMsg.buffer[2] = 'M';
    cpuStatus.LCDMsg.buffer[3] = ' ';
    cpuStatus.LCDMsg.buffer[4] = 'E';
    cpuStatus.LCDMsg.buffer[5] = 'R';
    cpuStatus.LCDMsg.buffer[6] = 'R';
    cpuStatus.LCDMsg.buffer[7] = 'O';
    cpuStatus.LCDMsg.buffer[8] = 'R';
    cpuStatus.LCDMsg.buffer[9] = 0x00;
    cpuStatus.LCDMsg.ct = 20;
    cpuStatus.LCDMsg.importanceLevel = 123;

    //segnalo ai miei subscriber che sono in com-error
    for (u32 i = 0; i < subscriberList.getNElem(); i++)
    {
        notify_CPU_STATE_CHANGED (subscriberList(i)->q, 0, logger, cpuStatus.VMCstate, cpuStatus.VMCerrorCode, cpuStatus.VMCerrorType);
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
    stato.set (sStato::eStato_normal, sStato::eWhatToDo_nothing);
}

/***************************************************
 *	priv_handleState_normal
 *
 *	Manda periodicamente il msg B (checkStatus) di richiesta di stato alla CPU.
 *	In caso di mancata risposta, passa allo stato com_error.
 *
 *	Se qualche subscriber invia il comando di startSelezione (e la selezione in questione � disponibile), allora si prova a passare in stato "selection"
 */
void Server::priv_handleState_normal()
{
	const u32 TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec = 250;
	const u8 ALLOW_N_RETRY_BEFORE_COMERROR = 2;
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
			}

			//schedulo il prossimo msg di stato
			nextTimeSendCheckStatusMsgWasMSec = rhea::getTimeNowMSec() + TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec;
		}

		//ci sono messaggi in ingresso?
		priv_handleMsgQueues (timeNowMSec, TIME_BETWEEN_ONE_STATUS_MSG_AND_ANOTHER_MSec);
	}
}


//***************************************************
void Server::priv_enterState_programmazione()
{
    logger->log("CPUBridgeServer::priv_enterState_programmazione()\n");
    stato.set (sStato::eStato_programmazione, sStato::eWhatToDo_nothing);
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
                    priv_enterState_normal();
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
 *	priv_parseAnswer_checkStatus
 *
 *	parsa la risposta della CPU alla mia richiesta di stato.
 *	Triggera alcuni eventi in modo da notificare chiunque sia in ascolta sulla coda di questo thread del fatto
 *	che qualcosa � cambiato.
 *	In particolare:
 *		CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED		se VMCstate/VMCerrorCode/VMCerrorType sono cambiati
 *		CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE	se � arrivato un msg LCD diverso dal precedente
 *		CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED		se il credito disponibile � cambiato
 *		CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED	se la disponibilit� delle selezioni � cambiata
 */
void Server::priv_parseAnswer_checkStatus (const u8 *answer, u16 answerLen UNUSED_PARAM)
{
	u8 isMultilangage = 0;
	const u8 prevMsgLcdCPUImportanceLevel = cpuStatus.LCDMsg.importanceLevel;
	cpuStatus.LCDMsg.importanceLevel = 0xff;

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

				//se il credito � cambiato, lo memorizzo e notifico i subscriber
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
					//1 byte per indicare se il btnProg � attualmente in stato di PRESSED
					u8 btnProgIsPressedNOW = answer[z++];
					if (btnProgIsPressedNOW == 1)
					{
						if (lastBtnProgStatus == 0)
						{
							//la CPU mi segnala che � stato premuto il btnPROG, diramo l'evento a tutti
							lastBtnProgStatus = 1;
							for (u32 i = 0; i < subscriberList.getNElem(); i++)
								notify_CPU_BTN_PROG_PRESSED(subscriberList(i)->q, 0, logger);
						}
					}
					else
					{
						lastBtnProgStatus = 0;
					}
					
				}
			}//if (cpuParamIniziali.protocol_version >= 3)
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
		una bevanda e, se si, se � in attesa che i sistemi di pagamento facciano il loro mestiere.
		Usiamo i bit 0x20 e 0x40 in questo modo:
			se == 0  => la CPU � una versione "vecchia" che non supporta questa nuova procedura
						per cui anche la GPU deve funzionare come faceva prima

			se == 0x01  =>  la CPU � nuova, 0x01 � il suo stato di default e vuol dire "non sto facendo nulla"
			se == 0x02  =>  la CPU ha capito che deve preparare una bevanda, � in attesa che i sistemi di pagamento rispondano
			se == 0x03  =>  la CPU ha dato l'OK alla preparazione (equivalente di BEVANDA IN PREP)
	*/
	cpuStatus.statoPreparazioneBevanda = (eStatoPreparazioneBevanda)((answer[9] & 0x60) >> 5);

    //u8 selection_CPU_current = answer[10];


	//messaggio testuale, pu� essere in ASCII o in unicode
	u16	msgLCD[LCD_BUFFER_SIZE_IN_U16];
	u8 msgLCDct = 0;
	u8 z = 11;
	//64 bytes unicode di messaggio "testuale"
	u16 firstGoodChar = ' ';
	for (u8 i = 0; i < 32; i++)
	{
		if (answer[1] == 'B')
			msgLCD[msgLCDct] = answer[z++];
		else //answer[1] == 'Z'
		{
			msgLCD[msgLCDct] = (u16)answer[z] + (u16)answer[z + 1] * 256;
			z += 2;
		}

		if (msgLCD[msgLCDct] != ' ' && firstGoodChar == ' ')
			firstGoodChar = msgLCD[msgLCDct];
		msgLCDct++;

		//mette uno spazio dopo i primi 16 char perch� storicamente il msg di CPU � composto da 2 messaggi da 16 char da visualizzare
		//uno sotto l'altro. Noi invece li visualizziamo sulla stessa riga
		if (msgLCDct == 16)
		{
			if (!isMultilangage || firstGoodChar != '@')
				msgLCD[msgLCDct++] = ' ';
		}
	}
	assert(msgLCDct <= LCD_BUFFER_SIZE_IN_U16);
	msgLCD[msgLCDct] = 0;
	
	//1 bit per ogni selezione per indicare se la selezione � disponibile o no
	//Considerando che NumMaxSelections=48, dovrebbero servire 6 byte
	//ATTENZIONE che bit==0 significa che la selezione � OK, bit==1 significa KO
	//Io invece traduco al contrario, per cui per me cupStatus.selAvailability == 1 se la selezione � disponibile
	u8 anythingChanged = 0;
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
		//la CPU non � in uno stato valido per fare erogazioni, per cui forzo a priori  la totalte
		//indisponibilit� delle bevande
		if (cpuStatus.selAvailability.areAllNotAvail() == false)
		{
			cpuStatus.selAvailability.reset();
			anythingChanged = 1;
		}
	}
	if (anythingChanged)
	{
		for (u32 i = 0; i < subscriberList.getNElem(); i++)
			notify_CPU_SEL_AVAIL_CHANGED (subscriberList(i)->q, 0, logger, &cpuStatus.selAvailability);
	}

	//se non c'� nemmeno una selezione disponibile, mostro sempre e cmq il msg di CPU anche se non fosse "importante"
	if (cpuStatus.selAvailability.areAllNotAvail())
		cpuStatus.LCDMsg.importanceLevel = 0xff;

	
	//se il messaggio LCD � cambiato dal giro precedente, oppure lo stato di importanza � cambiato...
	msgLCDct *= 2;
	if (prevMsgLcdCPUImportanceLevel != cpuStatus.LCDMsg.importanceLevel || msgLCDct != lastCPUMsgLen || memcmp(msgLCD, lastCPUMsg, msgLCDct) != 0)
	{
		memcpy (lastCPUMsg, msgLCD, msgLCDct);
		lastCPUMsgLen = msgLCDct;

		cpuStatus.LCDMsg.ct = msgLCDct;
		memcpy (cpuStatus.LCDMsg.buffer, msgLCD, msgLCDct);

		//lo traduco se necessario
		if (isMultilangage)
		{
			//se il primo ch diverso da "spazio" � "@", allora stiamo parlano di un messaggio custom
			u16 i = 0;
			while (lastCPUMsg[i] != 0x00)
			{
				if (lastCPUMsg[i] != ' ')
					break;
				++i;
			}
			if (lastCPUMsg[i] == LANG_CHIOCCIOLA)
			{
				u16 t = 0;
				while (lastCPUMsg[i] != 0x00)
					cpuStatus.LCDMsg.buffer[t++] = lastCPUMsg[i++];
				cpuStatus.LCDMsg.buffer[t] = 0x00;

				//rtrim
				if (t > 0)
				{
					--t;
					while (cpuStatus.LCDMsg.buffer[t] == ' ')
						cpuStatus.LCDMsg.buffer[t--] = 0x00;

					cpuStatus.LCDMsg.ct = lang_translate (&language, cpuStatus.LCDMsg.buffer, TRANSLATED_LCD_BUFFER_SIZE_IN_U16 -1);
					assert(cpuStatus.LCDMsg.ct < TRANSLATED_LCD_BUFFER_SIZE_IN_U16);
					cpuStatus.LCDMsg.ct *= 2;
				}
			}

		}

		//noticico tutti
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


    stato.set (sStato::eStato_selection, sStato::eWhatToDo_nothing);
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

	//una volta che la CPU � entrata in "PREPARAZIONE", deve tornare disponibile entro il tempo definito qui sotto
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



		/* Teoricamente l'intero processo � ora pilotato da "statoPreparazioneBevanda"
		 * Voglio per� essere sicuro di aver letto almeno una volta uno stato != da eStatoPreparazioneBevanda_doing_nothing
		 * prima di imbarcarmi nel processo.
		 * Appena vedo uno stato != da eStatoPreparazioneBevanda_doing_nothing setto bBevandaInPreparazione e parto.
		 * Se non vedo questa condizione entro 4/5 second, vuol dire che c'� qualcosa che non va e abortisco*/
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
					logger->log("priv_handleState_selection() => aborting, ero in WAIT ma CPU � andata in uno stato != da DISP o PREP");
					priv_onSelezioneTerminataKO();
					return;
				}
				else
				{
					runningSel.status = eRunningSelStatus_wait;
					if (runningSel.sub)
						//informo il subscriber che ha richiesto la bevanda che la CPU � ancora in wait
						notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
				}
				break;

			case eStatoPreparazioneBevanda_running:
				//la cpu ha dato l'OK, sta preparando la bevanda
				if (bBevandaInPreparazione == 1)
				{
					//� la prima volta che passo di qui
					bBevandaInPreparazione = 2;
					timeStartedMSec = timeNowMSec;

					if (cpuStatus.bShowDialogStopSelezione)
						runningSel.status = eRunningSelStatus_runningCanUseStopBtn;
					else
						runningSel.status = eRunningSelStatus_running;

					//la selezione � in preparazione, mando la notifica al subscriber che ha richiesto la bevanda
					if (runningSel.sub)
						notify_CPU_RUNNING_SEL_STATUS (runningSel.sub->q, 0, logger, runningSel.status);
				}

				//timeout di sicurezza. Diamo per scontato che una bevanda non possa durare pi� di TIMEOUT_SELEZIONE_2_MSEC
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

