#include "AlypayChina.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaDateTime.h"

using namespace rhea;



//***************************************************
AlipayChina::AlipayChina()
{
	localAllocator = NULL;
	logger = &nullLogger;
	rcvBuffer = NULL;
}

//***************************************************
void AlipayChina::useLogger(rhea::ISimpleLogger *loggerIN)
{
	if (NULL == loggerIN)
		logger = &nullLogger;
	else
		logger = loggerIN;
}

//***************************************************
void AlipayChina::close ()
{
	logger->log("AlipayChina::close\n");
	if (NULL == localAllocator)
		return;

	thread::deleteMsgQ (hMsgQRead, hMsgQWrite);
	
	RHEAFREE(localAllocator, rcvBuffer);
	RHEADELETE (rhea::memory_getDefaultAllocator(), localAllocator);
}


//***************************************************
bool AlipayChina::setup (const char *serverIPIN, u16 serverPortIN, const char *machineIDIN, const char *cryptoKeyIN, HThreadMsgW *out_hWrite)
{
	logger->log("AlipayChina::setup\n");
	logger->incIndent();

	localAllocator = RHEANEW(rhea::memory_getDefaultAllocator(), rhea::AllocatorSimpleWithMemTrack) ("AlipayChina");

	strcpy_s (serverIP, sizeof(serverIP), serverIPIN);
	serverPort = serverPortIN;
	strcpy_s (machineID, sizeof(machineID), machineIDIN);
	strcpy_s (cryptoKey, sizeof(cryptoKey), cryptoKeyIN);

	bIsLoggedIntoChinaServer = false;

	//creazione delle msgQ
	rhea::thread::createMsgQ (&hMsgQRead, &hMsgQWrite);
	*out_hWrite = hMsgQWrite;

	rcvBuffer = (u8*)RHEAALLOC(localAllocator, SIZE_OF_RCV_BUFFER);

	logger->log ("OK\n");
	logger->decIndent();
	return true;
}

//***************************************************
bool AlipayChina::priv_openSocket (OSSocket *sok)
{
	//apertura della socket
	logger->log("opening socket at %s:%d\n", serverIP, serverPort);
	
	eSocketError err = socket::openAsTCPClient (sok, serverIP, serverPort);
	if (eSocketError_none != err)
	{
		logger->log("error [%d]\n", (u32)err);
		return false;
	}
	return true;
}

//***************************************************
void AlipayChina::run ()
{
	const u32 SEND_HEARTBEAT_EVERY_MSEC = 30000;
	const u32 THIS_SOCKET_WAITID = 0x00000001;
	const u32 MSGQ_WAITID = 0x00000002;
	
	OSWaitableGrp waitableGrp;
	OSEvent hMsgQEvent;
	rhea::thread::getMsgQEvent (hMsgQRead, &hMsgQEvent);
	waitableGrp.addEvent (hMsgQEvent, MSGQ_WAITID);

	bool bIsSocketConnected = false;
	u64 nextTimeSendHeartBeatMSec = 0;
	OSSocket sok;
	socket::init(&sok);
	bQuit = false;
	while (bQuit == false)
	{
		//se necessario, prova ad aprire la socket
		if (!bIsSocketConnected)
		{
			logger->log("AlipayChina::run() => opening socket\n");
			logger->incIndent();
			
			bIsLoggedIntoChinaServer = false;
			nextTimeSendHeartBeatMSec = 0;
			nInRCVBuffer = 0;
			lastTimeHeartBeatRCVMSec = 0;
			theOrder.status = AlipayChina::eOrderStatus_none;
			
			bIsSocketConnected = priv_openSocket (&sok);
			if (bIsSocketConnected)
			{
				logger->log("OK\n");
				waitableGrp.addSocket (sok, THIS_SOCKET_WAITID);
			}
			else
			{
				logger->log("sleeping for a while...\n");
				thread::sleepMSec (5000);
			}
			logger->decIndent();
		
			continue;
		}

		//se arriviamo qui, vuol dire che la socket è connessa
		u8 nEvents = waitableGrp.wait (PAYMENT_POLL_MSec);
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		if (!bIsLoggedIntoChinaServer)
		{
			//provo a riloggarmi ogni "nextTimeSendHeartBeatMSec" msec
			if (timeNowMSec >= nextTimeSendHeartBeatMSec)
			{
				nextTimeSendHeartBeatMSec = timeNowMSec + SEND_HEARTBEAT_EVERY_MSEC;
				theOrder.status = AlipayChina::eOrderStatus_none;
				priv_sendLogin(sok);
			}
		}
		else
		{
			//heartbeat se serve
			if (timeNowMSec >= nextTimeSendHeartBeatMSec)
			{
				nextTimeSendHeartBeatMSec = timeNowMSec + SEND_HEARTBEAT_EVERY_MSEC;
				priv_sendHeartBeat(sok);
			}

			//se non ho ricevuto risposta all'heartbeat da un po' di tempo, disconnetto e riconnetto
			if (lastTimeHeartBeatRCVMSec > 0 && timeNowMSec - lastTimeHeartBeatRCVMSec > 65000)
			{
				logger->log ("AlipayChina::run() => last heartbeat was rcv more than [%d] ms ago, disconnecting\n", (u32)(timeNowMSec - lastTimeHeartBeatRCVMSec));
				bIsSocketConnected = false;
				theOrder.status = AlipayChina::eOrderStatus_none;
				waitableGrp.removeSocket(sok);
				socket::close(sok);
			}


			//se ho un ordine in attesa di pagamento, pollo il server
			if (theOrder.status == AlipayChina::eOrderStatus_pollingPaymentStatus)
			{
				if (timeNowMSec >= theOrder.timeoutMSec)
				{
					//l'utente non ha pagato nel tempo prestabilito, termino l'ordine
					logger->log ("Order timed out, user did not pay in time, order aborted\n");
					priv_orderClose (sok, AlipayChina::eOrderCloseStatus_TIMEOUT);
					theOrder.status = AlipayChina::eOrderStatus_none;
				}
				else if (timeNowMSec >= theOrder.nextTimePollPaymentoMSec)
				{
					logger->log ("polling for payment status\n");
					priv_orderAskForPaymentStatus(sok);
					theOrder.nextTimePollPaymentoMSec += PAYMENT_POLL_MSec;
				}
			}
			else if (theOrder.status == AlipayChina::eOrderStatus_paymentOK)
			{
				//l'utente ha pagato, ora aspetto che qualcuno mi comunichi se la bevanda è stata servita con successo
				logger->log ("Order [%s] has been paid, now waiting to know when beverage has been delivered\n", theOrder.orderNumber);
				theOrder.status = AlipayChina::eOrderStatus_waitingBeverageEnd;
				theOrder.timeoutMSec = rhea::getTimeNowMSec() + 120000;
			}
			else if (theOrder.status == AlipayChina::eOrderStatus_waitingBeverageEnd)
			{
				//sono in attesa che la selezione termini e che qualcuno mi dica il risultato dell'erogazione
				if (timeNowMSec >= theOrder.timeoutMSec)
				{
					//la selezione ci ha messo troppo, qualcosa non va, abortisco l'ordine
					logger->log ("Order timed out while waiting for beverage ends, order aborted\n");
					priv_orderClose (sok, AlipayChina::eOrderCloseStatus_TIMEOUT);
					theOrder.status = AlipayChina::eOrderStatus_none;
				}
			}
			else if (theOrder.status == AlipayChina::eOrderStatus_finishedOK)
			{
				logger->log ("Order [%s] finished, SUCCESS\n", theOrder.orderNumber);
				priv_orderClose (sok, AlipayChina::eOrderCloseStatus_SUCCESS);
				theOrder.status = AlipayChina::eOrderStatus_none;
			}
			else if (theOrder.status == AlipayChina::eOrderStatus_finishedKO)
			{
				logger->log ("Order [%s] finished, FAIL\n", theOrder.orderNumber);
				priv_orderClose (sok, AlipayChina::eOrderCloseStatus_FAILED);
				theOrder.status = AlipayChina::eOrderStatus_none;
			}
		}
	


		//gestione degli eventi
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			default:
				logger->log ("AlipayChina::run(), unknown event origin: %d\n", waitableGrp.getEventOrigin(i));
				break;

			case OSWaitableGrp::evt_origin_deleted:
				break;

			case OSWaitableGrp::evt_origin_socket:
				if (waitableGrp.getEventUserParamAsU32(i) == THIS_SOCKET_WAITID)
				{
					if (bIsSocketConnected)
					{
						if (!priv_handleIncomingSocketMsg(sok))
						{
							bIsSocketConnected = false;
							waitableGrp.removeSocket(sok);
							socket::close(sok);
						}
					}
				}
				else
				{
					DBGBREAK;
					logger->log ("AlipayChina::run(), unknown socket msg received\n");
				}
				break;

			case OSWaitableGrp::evt_origin_osevent:
				if (waitableGrp.getEventUserParamAsU32(i) == MSGQ_WAITID)
					priv_handleIncomingThreadMsg();
				else
				{
					DBGBREAK;
					logger->log ("AlipayChina::run(), unknown thread msg received\n");
				}
				break;
			}
		}
	}


	waitableGrp.removeEvent (hMsgQEvent);
	waitableGrp.removeSocket(sok);
}

//***************************************************
bool AlipayChina::priv_handleIncomingSocketMsg(OSSocket &sok)
{
	while (1)
	{
		const i32 nRead = socket::read (sok, &rcvBuffer[nInRCVBuffer], SIZE_OF_RCV_BUFFER - nInRCVBuffer, 0, false);
		if (0 == nRead)
		{
			//socket disconnessa!!
			nInRCVBuffer = 0;
			return false;
		}

		if (-1 == nRead)
		{
			//chiamata bloccante, aspetto un po' e riprovo
			thread::sleepMSec(1000);
			continue;
		}

		nInRCVBuffer += nRead;
		while (nInRCVBuffer)
		{
			sResponse response;
			const u32 nBytesUsed = priv_parserRCVCommand (rcvBuffer, nInRCVBuffer, &response);

			if (nBytesUsed == 0)
			{
				//nel buffer non c'erano abbastanza dati per un messaggio completo
				//Lascio nel buffer quel che c'era  e aspetto futuri aggiornamenti
				return true;
			}

			//ho consumato parte del buffer, elimino la parte utilizzata e ripeto
			assert (nInRCVBuffer >= nBytesUsed);
			nInRCVBuffer -= nBytesUsed;
			if (nInRCVBuffer > 0)
				memcpy (rcvBuffer, &rcvBuffer[nBytesUsed], nInRCVBuffer);

			//elaboro il comando
			switch (response.command)
			{
			case AlipayChina::eResponseCommand_none:
				break;

			case AlipayChina::eResponseCommand_E11:	//login
				logger->log ("AlipayChina::priv_handleIncomingSocketMsg() => rcv E11 response, status [%c]\n", response.asE11.status);
				if (response.asE11.status == '0')
				{
					bIsLoggedIntoChinaServer = true;
					lastTimeHeartBeatRCVMSec = rhea::getTimeNowMSec();
				}
				else
				{
					logger->log ("login failed\n");
					bIsLoggedIntoChinaServer = false;
					theOrder.status = AlipayChina::eOrderStatus_none;
				}
				break;

			case AlipayChina::eResponseCommand_E12:	//heartbeat
				logger->log ("AlipayChina::priv_handleIncomingSocketMsg() => rcv E12 response, status [%c]\n", response.asE12.status);
				lastTimeHeartBeatRCVMSec = rhea::getTimeNowMSec();
				break;

			case AlipayChina::eResponseCommand_E13: //risposta a priv_orderStart()
				logger->log ("AlipayChina::priv_handleIncomingSocketMsg() => rcv E13, [%s] [%s] [%s]\n", response.asE13.orderNumber, response.asE13.qrCodeURL, response.asE13.paymentMethod);

				if (response.asE13.orderNumber[0] == 0x00)
				{
					//il server ha generato un errore, devo abortire l'ordine
					logger->log ("Server error while generating QR, aborting order\n");
					theOrder.status = AlipayChina::eOrderStatus_none;
				}
				else
				{
					//tutto ok, da ora in poi devo cominciare a pollare per verificare il pagamento
					strcpy_s (theOrder.orderNumber, sizeof(theOrder.orderNumber), response.asE13.orderNumber);
					strcpy_s (theOrder.qrCodeURL, sizeof(theOrder.qrCodeURL), response.asE13.qrCodeURL);
					strcpy_s (theOrder.acceptedPaymentMethods, sizeof(theOrder.acceptedPaymentMethods), response.asE13.paymentMethod);
					theOrder.timeoutMSec = rhea::getTimeNowMSec() + TIMEOUT_FOR_PAYMENT_MSec;
					theOrder.nextTimePollPaymentoMSec = rhea::getTimeNowMSec() + PAYMENT_POLL_MSec;
					theOrder.status = AlipayChina::eOrderStatus_pollingPaymentStatus;
				}
				break;

			case AlipayChina::eResponseCommand_E14: //risposta a priv_orderAskForPaymentStatus()
				logger->log ("AlipayChina::priv_handleIncomingSocketMsg() => rcv E14 response, status [%d], order [%s]\n", response.asE14.status, response.asE14.orderNumber);
				if (response.asE14.status == '3')
					theOrder.status = AlipayChina::eOrderStatus_paymentOK;
				break;

			case AlipayChina::eResponseCommand_E15: //risposta a priv_orderClose()
				logger->log ("AlipayChina::priv_handleIncomingSocketMsg() => rcv E15 response, status [%d], order [%s]\n", response.asE15.status, response.asE15.orderNumber);
				theOrder.status = AlipayChina::eOrderStatus_none;
				break;
			}
		}

		return true;
	}
}

//***************************************************
u32 AlipayChina::priv_parserRCVCommand (const u8 *buffer, u32 sizeOfBuffer, sResponse *out)
{
	out->command = AlipayChina::eResponseCommand_none;
	
	//La risposta del server inizia sempre con
	// machineID|CCC|YYYYMMDDHHMMSS|....#
	const u32 sizeOfMachineID = strlen(this->machineID);
	const u32 minSizeOfAMsg = sizeOfMachineID + 1 + 3 + 1 + 14 + 1 +1;
	if (sizeOfBuffer < minSizeOfAMsg)
		return 0;

	if (strncasecmp((const char*)buffer, this->machineID, sizeOfMachineID) != 0)
	{
		//non va bene, siamo nel mezzo di un messaggio sporco.
		//consumo 1 byte e riprovo
		return 1;
	}

	//subito dopo machineCode ci deve essere "|"
	u32 n = sizeOfMachineID;
	if (buffer[n++] != '|')
		return sizeOfMachineID;

	//a seguire abbiamo il comando
	char command[4];
	command[0] = buffer[n++];
	command[1] = buffer[n++];
	command[2] = buffer[n++];
	command[3] = 0x00;

	//subito dopo ci deve essere "|"
	if (buffer[n++] != '|')
		return sizeOfMachineID;

	//a seguire abbiamo il timestamp
	memcpy (out->timestamp, &buffer[n], 14);
	out->timestamp[14] = 0x00;
	n += 14;

	//subito dopo ci deve essere "|"
	if (buffer[n++] != '|')
		return sizeOfMachineID;

	//ok, in base al comando ricevuto, i dati che seguono possono variare
	if (strncasecmp(command, "E11", 3) == 0)
	{
		//C20190001 | E11 | 20191029223012 | 0 #
		if (sizeOfBuffer < minSizeOfAMsg + 1)
			return 0;
		out->command = AlipayChina::eResponseCommand_E11;
		out->asE11.status = buffer[n++];
	}
	else if (strncasecmp(command, "E12", 3) == 0)
	{
		//C20190001 | E12 | 20191029223012 | 0 #
		if (sizeOfBuffer < minSizeOfAMsg + 1)
			return 0;
		out->command = AlipayChina::eResponseCommand_E12;
		out->asE12.status = buffer[n++];
	}
	else if (strncasecmp(command, "E13", 3) == 0)
	{
		//C20190001 | E13 | 20191029223012 | OrderNumber | QRCodeURL | PaymentMethod #
		//I dati successivi sono a lunghezza variabile.
		//Devo trovare almeno due "|" e poi un "#"
		
		//C'è il caso particolare in cui al posto di "orderNumber" si trova un "EE|". Questo vuol dire che c'è stato un errore sul server,
		//bisogna abortire l'operazione
		if (buffer[n] == 'E' && buffer[n + 1] == 'E' && buffer[n + 2] == '|')
		{
			//cerco # di fine messaggi e ritorno
			out->command = AlipayChina::eResponseCommand_E13;
			out->asE13.orderNumber[0] = 0x00;
			while (n < sizeOfBuffer)
			{
				if (buffer[n] == '#')
					break;
				++n;
			}
		}
		else
		{
			u32 ct = n;
			u32 pipe1Found = 0;
			u32 pipe2Found = 0;
			u32 sharpFound = 0;
			while (ct < sizeOfBuffer)
			{
				if (buffer[ct] == '|')
				{
					if (pipe1Found == 0)
						pipe1Found = ct;
					else
						pipe2Found = ct;
				}
				else if (buffer[ct] == '#')
				{
					sharpFound = ct;
					break;
				}
				++ct;
			}

			if (!pipe1Found || !pipe2Found || !sharpFound)
				return 0;
			if (pipe2Found >= sharpFound)
				return 0;

			out->command = AlipayChina::eResponseCommand_E13;

			memcpy (out->asE13.orderNumber, &buffer[n], pipe1Found - n);
			out->asE13.orderNumber[pipe1Found - n] = 0x00;

			n = pipe1Found + 1;
			memcpy (out->asE13.qrCodeURL, &buffer[n], pipe2Found - n);
			out->asE13.qrCodeURL[pipe2Found - n] = 0x00;

			n = pipe2Found + 1;
			memcpy (out->asE13.paymentMethod, &buffer[n], sharpFound - n);
			out->asE13.paymentMethod[sharpFound - n] = 0x00;

			n = sharpFound;
		}
	}
	else if (strncasecmp(command, "E14", 3) == 0)
	{
		//machineID   cmd   timestamp        orderNumber           status
		//C20190001 | E14 | 20191029223010 | 2019102922401366789 | 3 #
		//
		//status == 1 => Order Initialization
		//status == 2 => Consumer has Scanned QR Code
		//status == 3 => Payment Succeeded
		u32 ct = n;
		u32 pipeFound = 0;
		u32 sharpFound = 0;
		while (ct < sizeOfBuffer)
		{
			if (buffer[ct] == '|')
				pipeFound = ct;
			else if (buffer[ct] == '#')
			{
				sharpFound = ct;
				break;
			}
			++ct;
		}

		if (!pipeFound || !sharpFound)
			return 0;
		if (pipeFound >= sharpFound)
			return 0;

		out->command = AlipayChina::eResponseCommand_E14;

		memcpy (out->asE14.orderNumber, &buffer[n], pipeFound - n);
		out->asE14.orderNumber[pipeFound - n] = 0x00;

		n = pipeFound + 1;
		out->asE14.status = buffer[n++];
	}
	else if (strncasecmp(command, "E15", 3) == 0) //risposta a priv_orderClose()
	{
		//machineID   cmd   timestamp        orderNumber           status
		//C20190001 | E15 | 20191029223010 | 2019102922401366789 | 0 #
		//
		//status == 0 => Success
		//status == 1 => fail
		u32 ct = n;
		u32 pipeFound = 0;
		u32 sharpFound = 0;
		while (ct < sizeOfBuffer)
		{
			if (buffer[ct] == '|')
				pipeFound = ct;
			else if (buffer[ct] == '#')
			{
				sharpFound = ct;
				break;
			}
			++ct;
		}

		if (!pipeFound || !sharpFound)
			return 0;
		if (pipeFound >= sharpFound)
			return 0;

		out->command = AlipayChina::eResponseCommand_E15;

		memcpy (out->asE15.orderNumber, &buffer[n], pipeFound - n);
		out->asE15.orderNumber[pipeFound - n] = 0x00;

		n = pipeFound + 1;
		out->asE15.status = buffer[n++];
	}
	else
	{
		//comando non riconosciuto
		logger->log ("AlipayChina::priv_parserRCVCommand() => command [%s] not handled\n", out->command);
		out->command = AlipayChina::eResponseCommand_none;
		return minSizeOfAMsg;
	}

	//l'ultimo char deve essere un "#"
	if (buffer[n++] != '#')
		return sizeOfMachineID;


#ifdef _DEBUG
	logger->log ("RCV: ");
	for (u32 i = 0; i < n; i++)
		logger->log ("%c", buffer[i]);
	logger->log ("\n");
#endif

	return n;
}

//***************************************************
void AlipayChina::priv_handleIncomingThreadMsg()
{}



//***************************************************
void AlipayChina::priv_sendLogin(OSSocket &sok)
{
	logger->log ("AlipayChina::priv_sendLogin()\n");
	const char API_VERSION[] = {"V1.6"};
	const char COMMAND[] = { "E11" };

	char timestamp[16];
	priv_getTimestamp (timestamp, sizeof(timestamp));
	
	char s[512];
	sprintf_s (s, sizeof(s), "%s|%s|%s|%s%s", this->machineID, COMMAND, timestamp, API_VERSION, this->cryptoKey);

	char hashedKey[64];
	rhea::utils::md5 (hashedKey, sizeof(hashedKey), s, (u32)strlen(s));

	sprintf_s (s, sizeof(s), "%s|%s|%s|%s|%s#", this->machineID, COMMAND, timestamp, API_VERSION, hashedKey);
	priv_sendCommand (sok, (u8*)s, strlen(s));	
}

//***************************************************
void AlipayChina::priv_sendHeartBeat(OSSocket &sok)
{
	logger->log ("AlipayChina::priv_sendHeartBeat()\n");
	u8 buffer[128];
	const u32 nBytes = priv_buildCommand ("E12", (const u8*)"0", buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytes);
}

//***************************************************
void AlipayChina::priv_getTimestamp(char *out, u32 sizeofOut) const
{
	rhea::DateTime dt;
	dt.setNow();
	dt.formatAs_YYYYMMDDHHMMSS (out, sizeofOut, 0x00, 0x00, 0x00);
}


/***************************************************
 * invia richiesta al server per iniziare un ordine.
 * Se tutto va bene, il server risponde con un E13 indicando l'URL da embeddare nel QR code.
 * Da li in poi, si polla il server con E14 per conoscere lo stato della scansione del QR code da parte dell'utente.
 * L'ordine infine si chiude con un E15 comunicando al server l'esito della selezione
 */
bool AlipayChina::priv_orderStart (OSSocket &sok, const u8 *selectionName, u8 selectionNum, const char *selectionPrice)
{
	logger->log ("AlipayChina::priv_orderStart()\n");
	if (theOrder.status != AlipayChina::eOrderStatus_none)
	{
		logger->incIndent();
		logger->log ("ERR: theOrder.status != none [status=%d]\n", (u8)theOrder.status);
		logger->decIndent();
		return false;
	}

	sprintf_s ((char*)theOrder.param_selectionName, sizeof(theOrder.param_selectionName), "%s", selectionName);
	theOrder.param_selectionNumber = selectionNum;
	sprintf_s (theOrder.param_selectionPrice, sizeof(theOrder.param_selectionPrice), "%s", selectionPrice);
	theOrder.orderNumber[0] = 0x00;
	theOrder.status = AlipayChina::eOrderStatus_requestSent;

	u8 buffer[512];
	char s[256];
	sprintf_s (s, sizeof(s), "%s|%d|%s", selectionName, selectionNum, selectionPrice);
	const u32 nBytesToSend = priv_buildCommand ("E13", (const u8*)s, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);
	return true;
}

//***************************************************
void AlipayChina::priv_orderAskForPaymentStatus (OSSocket &sok)
{
	u8 buffer[128];
	const u32 nBytesToSend = priv_buildCommand ("E14", (const u8*)theOrder.orderNumber, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);
}

//***************************************************
void AlipayChina::priv_orderClose (OSSocket &sok, eOrderCloseStatus status)
{
	char s[128];
	sprintf_s (s, sizeof(s), "%s|%d", theOrder.orderNumber, (u8)status);

	u8 buffer[128];
	const u32 nBytesToSend = priv_buildCommand ("E15", (const u8*)s, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);

	theOrder.status = AlipayChina::eOrderStatus_none;
}

//***************************************************
void AlipayChina::onSelectionFinished(bool bSelectionWasDelivered)
{
	if (theOrder.status != AlipayChina::eOrderStatus_waitingBeverageEnd)
	{
		logger->log ("AlipayChina::onSelectionFinished() => error, order was in wrong status [%d]\n", (u8)theOrder.status);
		return;
	}

	if (bSelectionWasDelivered)
		theOrder.status = eOrderStatus_finishedOK;
	else
		theOrder.status = eOrderStatus_finishedKO;
}

//***************************************************
u32 AlipayChina::priv_buildCommand (const char *command, const u8 *optionalData, u8 *out_buffer, u32 sizeofOutBuffer) const
{
	char timestamp[16];
	priv_getTimestamp (timestamp, sizeof(timestamp));

	sprintf_s ((char*)out_buffer, sizeofOutBuffer, "%s|%s|%s", this->machineID, command, timestamp);
	if (NULL != optionalData)
	{
		strcat_s ((char*)out_buffer, sizeofOutBuffer, "|");
		strcat_s ((char*)out_buffer, sizeofOutBuffer, (const char*)optionalData);
	}
	strcat_s ((char*)out_buffer, sizeofOutBuffer, "#");

	return (u32)strlen((char*)out_buffer);
}

//***************************************************
void AlipayChina::priv_sendCommand (OSSocket &sok, const u8 *buffer, u32 nBytesToSend)
{
#ifdef _DEBUG
	logger->log ("SND: ");
	for (u32 i = 0; i < nBytesToSend; i++)
		logger->log ("%c", buffer[i]);
	logger->log ("\n");
#endif
	
	if (nBytesToSend > 0)
		socket::write (sok, buffer, nBytesToSend);
}