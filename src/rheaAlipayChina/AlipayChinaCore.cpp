#include "AlipayChinaEnumAndDefine.h"
#include "AlipayChinaCore.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaLogTargetConsole.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaDateTime.h"

using namespace rhea;
using namespace rhea::AlipayChina;



//***************************************************
Core::Core()
{
	localAllocator = NULL;
	logger = &nullLogger;
	rcvBuffer = NULL;
}

//***************************************************
void Core::useLogger(rhea::ISimpleLogger *loggerIN)
{
	if (NULL == loggerIN)
		logger = &nullLogger;
	else
		logger = loggerIN;
}

//***************************************************
bool Core::setup (const char *serverIPIN, u16 serverPortIN, const char *machineIDIN, const char *cryptoKeyIN, HThreadMsgW *out_hWrite)
{
	logger->log("AlipayChinaCore::setup [ip:%s:%d] [machineID:%s] [key:%s]\n", serverIPIN, serverPortIN, machineIDIN, cryptoKeyIN);
	logger->incIndent();

	localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("AlipayChinaCore");

	strcpy_s (serverIP, sizeof(serverIP), serverIPIN);
	serverPort = serverPortIN;
	strcpy_s (machineID, sizeof(machineID), machineIDIN);
	strcpy_s (cryptoKey, sizeof(cryptoKey), cryptoKeyIN);

	bIsLoggedIntoChinaServer = false;

	//creazione delle msgQ
	rhea::thread::createMsgQ (&hMsgQRead, &hMsgQWrite);
	*out_hWrite = hMsgQWrite;

	rcvBuffer = (u8*)RHEAALLOC(localAllocator, SIZE_OF_RCV_BUFFER);

	toBeNotifiedThreadList.setup (localAllocator, 4);

	logger->log ("OK\n");
	logger->decIndent();
	return true;
}

//***************************************************
bool Core::priv_openSocket (OSSocket *sok)
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
void Core::run ()
{
	const u32 SEND_HEARTBEAT_EVERY_MSEC = 30000;
	const u32 THIS_SOCKET_WAITID = 0x00000001;
	const u32 MSGQ_WAITID = 0x00000002;
	
	OSWaitableGrp waitableGrp;
	OSEvent hMsgQEvent;
	rhea::thread::getMsgQEvent (hMsgQRead, &hMsgQEvent);
	waitableGrp.addEvent (hMsgQEvent, MSGQ_WAITID);

    OSSocket sok;
    socket::init(&sok);

    //attendo che qualcuno si iscriva a me
    bool bAtLeastOnSubscriber = false;
    while (bAtLeastOnSubscriber == false)
    {
        u8 nEvents = waitableGrp.wait (PAYMENT_POLL_MSec);
        for (u8 i = 0; i < nEvents; i++)
        {
            switch (waitableGrp.getEventOrigin(i))
            {
            default:
                break;

            case OSWaitableGrp::evt_origin_osevent:
                if (waitableGrp.getEventUserParamAsU32(i) == MSGQ_WAITID)
                {
                    priv_handleIncomingThreadMsg(sok);
                    if (toBeNotifiedThreadList.getNElem())
                        bAtLeastOnSubscriber=true;
                }
                break;
            }
        }
    }



    memset (&theOrder, 0, sizeof(theOrder));
	theOrder.status = Core::eOrderStatus_none;

	bool bIsSocketConnected = false;
	u64 nextTimeSendHeartBeatMSec = 0;
	bQuit = false;
	while (bQuit == false)
	{
		//se necessario, prova ad aprire la socket
		if (!bIsSocketConnected)
		{
			logger->log("AlipayChinaCore::run() => opening socket\n");
			logger->incIndent();
			
			bIsLoggedIntoChinaServer = false;
			nextTimeSendHeartBeatMSec = 0;
			nInRCVBuffer = 0;
			lastTimeHeartBeatRCVMSec = 0;

			if (theOrder.status != Core::eOrderStatus_none)
			{
				priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
				priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
			}
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

        //se arriviamo qui, vuol dire che la socket e' connessa
		u8 nEvents = waitableGrp.wait (PAYMENT_POLL_MSec);
		const u64 timeNowMSec = rhea::getTimeNowMSec();

		if (!bIsLoggedIntoChinaServer)
		{
			//provo a riloggarmi ogni "nextTimeSendHeartBeatMSec" msec
			if (timeNowMSec >= nextTimeSendHeartBeatMSec)
			{
				nextTimeSendHeartBeatMSec = timeNowMSec + SEND_HEARTBEAT_EVERY_MSEC;
				if (theOrder.status != Core::eOrderStatus_none)
				{
					priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
					priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
				}
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
				logger->log ("AlipayChinaCore::run() => last heartbeat was rcv more than [%d] ms ago, disconnecting\n", (u32)(timeNowMSec - lastTimeHeartBeatRCVMSec));
				if (theOrder.status != Core::eOrderStatus_none)
				{
					priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
					priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
				}
				bIsLoggedIntoChinaServer = false;
				bIsSocketConnected = false;
				waitableGrp.removeSocket(sok);
				socket::close(sok);
				
				priv_notify_ONLINE_STATUS_CHANGED();
			}


			//gestione dell'ordine in corso
			if (theOrder.status == Core::eOrderStatus_requestSent)
			{
				//ho chiesto il QR al server, sto aspettando risposta
				if (timeNowMSec >= theOrder.timeoutMSec)
				{
					logger->log ("Order timed out waiting for QR code\n");
					if (theOrder.status != Core::eOrderStatus_none)
					{
						priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
						priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
					}
				}
			}
			else if (theOrder.status == Core::eOrderStatus_pollingPaymentStatus)
			{
				//ho ricevuto il qr, ora pollo il server per sapere se l'utente ha pagato
				if (timeNowMSec >= theOrder.timeoutMSec)
				{
					//l'utente non ha pagato nel tempo prestabilito, termino l'ordine
					logger->log ("Order timed out, user did not pay in time, order aborted\n");
					if (theOrder.status != Core::eOrderStatus_none)
					{
						priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
						priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
					}
				}
				else if (timeNowMSec >= theOrder.nextTimePollPaymentoMSec)
				{
					logger->log ("polling for payment status\n");
					priv_orderAskForPaymentStatus(sok);
					theOrder.nextTimePollPaymentoMSec += PAYMENT_POLL_MSec;
				}
			}
			else if (theOrder.status == Core::eOrderStatus_paymentOK)
			{
				//l'utente ha pagato, ora aspetto che qualcuno mi comunichi se la bevanda Ã¨ stata servita con successo
				logger->log ("Order [%s] has been paid, now waiting to know when beverage has been delivered\n", theOrder.orderNumber);
				priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentOK);
				theOrder.status = Core::eOrderStatus_waitingBeverageEnd;
				theOrder.timeoutMSec = rhea::getTimeNowMSec() + 120000;
			}
			else if (theOrder.status == Core::eOrderStatus_waitingBeverageEnd)
			{
				//sono in attesa che la selezione termini e che qualcuno mi dica il risultato dell'erogazione
				if (timeNowMSec >= theOrder.timeoutMSec)
				{
					//la selezione ci ha messo troppo, qualcosa non va, abortisco l'ordine sul server
					logger->log ("Order timed out while waiting for beverage ends, order aborted\n");
					priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
				}
			}
			else if (theOrder.status == Core::eOrderStatus_finishedOK)
			{
				logger->log ("Order [%s] finished, SUCCESS\n", theOrder.orderNumber);
				priv_orderClose (sok, Core::eOrderCloseStatus_SUCCESS);
			}
			else if (theOrder.status == Core::eOrderStatus_finishedKO)
			{
				logger->log ("Order [%s] finished, FAIL\n", theOrder.orderNumber);
				priv_orderClose (sok, Core::eOrderCloseStatus_FAILED);
			}
		}
	


		//gestione degli eventi
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			default:
				logger->log ("AlipayChinaCore::run(), unknown event origin: %d\n", waitableGrp.getEventOrigin(i));
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
					logger->log ("AlipayChinaCore::run(), unknown socket msg received\n");
				}
				break;

			case OSWaitableGrp::evt_origin_osevent:
				if (waitableGrp.getEventUserParamAsU32(i) == MSGQ_WAITID)
					priv_handleIncomingThreadMsg(sok);
				else
				{
					DBGBREAK;
					logger->log ("AlipayChinaCore::run(), unknown thread msg received\n");
				}
				break;
			}
		}
	}


	logger->log("AlipayChinaCore::die\n");
	waitableGrp.removeEvent (hMsgQEvent);
	waitableGrp.removeSocket(sok);

	if (bIsSocketConnected)
		rhea::socket::close (sok);
	thread::deleteMsgQ (hMsgQRead, hMsgQWrite);
	RHEAFREE(localAllocator, rcvBuffer);
	toBeNotifiedThreadList.unsetup();
	RHEADELETE (rhea::getSysHeapAllocator(), localAllocator);
}

//***************************************************
bool Core::priv_handleIncomingSocketMsg(OSSocket &sok)
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
			case Core::eResponseCommand_none:
				break;

			case Core::eResponseCommand_E11:	//login
				logger->log ("AlipayChinaCore::priv_handleIncomingSocketMsg() => rcv E11 response, status [%c]\n", response.asE11.status);
				if (response.asE11.status == '0')
				{
					if (!bIsLoggedIntoChinaServer)
					{
						bIsLoggedIntoChinaServer = true;
						priv_notify_ONLINE_STATUS_CHANGED();
					}
					lastTimeHeartBeatRCVMSec = rhea::getTimeNowMSec();
					priv_notify_ONLINE_STATUS_CHANGED();
				}
				else
				{
					logger->log ("login failed\n");
					if (bIsLoggedIntoChinaServer)
					{
						bIsLoggedIntoChinaServer = false;
						priv_notify_ONLINE_STATUS_CHANGED();
					}
					
					theOrder.status = Core::eOrderStatus_none;
				}
				break;

			case Core::eResponseCommand_E12:	//heartbeat
				logger->log ("AlipayChinaCore::priv_handleIncomingSocketMsg() => rcv E12 response, status [%c]\n", response.asE12.status);
				lastTimeHeartBeatRCVMSec = rhea::getTimeNowMSec();
				break;

			case Core::eResponseCommand_E13: //risposta a priv_orderStart()
				logger->log ("AlipayChinaCore::priv_handleIncomingSocketMsg() => rcv E13, [%s] [%s] [%s]\n", response.asE13.orderNumber, response.asE13.qrCodeURL, response.asE13.paymentMethod);

				if (response.asE13.orderNumber[0] == 0x00)
				{
					//il server ha generato un errore, devo abortire l'ordine
					logger->log ("Server error while generating QR, aborting order\n");
					priv_orderClose (sok, Core::eOrderCloseStatus_TIMEOUT);
					priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentTimeout);
				}
				else
				{
					//tutto ok, da ora in poi devo cominciare a pollare per verificare il pagamento
					strcpy_s (theOrder.orderNumber, sizeof(theOrder.orderNumber), response.asE13.orderNumber);
					strcpy_s (theOrder.qrCodeURL, sizeof(theOrder.qrCodeURL), response.asE13.qrCodeURL);
					strcpy_s (theOrder.acceptedPaymentMethods, sizeof(theOrder.acceptedPaymentMethods), response.asE13.paymentMethod);
					theOrder.timeoutMSec = rhea::getTimeNowMSec() + TIMEOUT_FOR_PAYMENT_MSec;
					theOrder.nextTimePollPaymentoMSec = rhea::getTimeNowMSec() + PAYMENT_POLL_MSec;
					theOrder.status = Core::eOrderStatus_pollingPaymentStatus;
					
					priv_notify_ORDER_STATUS (AlipayChina::eOrderStatus_pollingPaymentStatus, theOrder.qrCodeURL, strlen(theOrder.qrCodeURL)+1);
				}
				break;

			case Core::eResponseCommand_E14: //risposta a priv_orderAskForPaymentStatus()
				logger->log ("AlipayChinaCore::priv_handleIncomingSocketMsg() => rcv E14 response, status [%d], order [%s]\n", response.asE14.status, response.asE14.orderNumber);
				if (response.asE14.status == '3')
				{
					theOrder.status = Core::eOrderStatus_paymentOK;
					priv_notify_ORDER_STATUS(AlipayChina::eOrderStatus_paymentOK);
				}
				break;

			case Core::eResponseCommand_E15: //risposta a priv_orderClose()
				logger->log ("AlipayChinaCore::priv_handleIncomingSocketMsg() => rcv E15 response, status [%d], order [%s]\n", response.asE15.status, response.asE15.orderNumber);
				theOrder.status = Core::eOrderStatus_none;
				break;
			}
		}

		return true;
	}
}

//***************************************************
u32 Core::priv_parserRCVCommand (const u8 *buffer, u32 sizeOfBuffer, sResponse *out)
{
	out->command = Core::eResponseCommand_none;
	
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
		out->command = Core::eResponseCommand_E11;
		out->asE11.status = buffer[n++];
	}
	else if (strncasecmp(command, "E12", 3) == 0)
	{
		//C20190001 | E12 | 20191029223012 | 0 #
		if (sizeOfBuffer < minSizeOfAMsg + 1)
			return 0;
		out->command = Core::eResponseCommand_E12;
		out->asE12.status = buffer[n++];
	}
	else if (strncasecmp(command, "E13", 3) == 0)
	{
		//C20190001 | E13 | 20191029223012 | OrderNumber | QRCodeURL | PaymentMethod #
		//I dati successivi sono a lunghezza variabile.
		//Devo trovare almeno due "|" e poi un "#"
		
		//C'Ã¨ il caso particolare in cui al posto di "orderNumber" si trova un "EE|". Questo vuol dire che c'Ã¨ stato un errore sul server,
		//bisogna abortire l'operazione
		if (buffer[n] == 'E' && buffer[n + 1] == 'E' && buffer[n + 2] == '|')
		{
			//cerco # di fine messaggi e ritorno
			out->command = Core::eResponseCommand_E13;
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

			out->command = Core::eResponseCommand_E13;

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

		out->command = Core::eResponseCommand_E14;

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

		out->command = Core::eResponseCommand_E15;

		memcpy (out->asE15.orderNumber, &buffer[n], pipeFound - n);
		out->asE15.orderNumber[pipeFound - n] = 0x00;

		n = pipeFound + 1;
		out->asE15.status = buffer[n++];
	}
	else
	{
		//comando non riconosciuto
		logger->log ("AlipayChinaCore::priv_parserRCVCommand() => command [%s] not handled\n", out->command);
		out->command = Core::eResponseCommand_none;
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
void Core::priv_sendLogin(OSSocket &sok)
{
	logger->log ("AlipayChinaCore::priv_sendLogin()\n");
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
void Core::priv_sendHeartBeat(OSSocket &sok)
{
	logger->log ("AlipayChinaCore::priv_sendHeartBeat()\n");
	u8 buffer[128];
	const u32 nBytes = priv_buildCommand ("E12", (const u8*)"0", buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytes);
}

//***************************************************
void Core::priv_getTimestamp(char *out, u32 sizeofOut) const
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
bool Core::priv_orderStart (OSSocket &sok, const u8 *selectionName, u8 selectionNum, const char *selectionPrice)
{
	logger->log ("AlipayChinaCore::priv_orderStart()\n");
	if (theOrder.status != Core::eOrderStatus_none)
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
	theOrder.status = Core::eOrderStatus_requestSent;
	theOrder.timeoutMSec = rhea::getTimeNowMSec() + 3000;

	u8 buffer[512];
	char s[256];

    //2020-12-01
    //I cinesi lamentano il fatto che "alcune selezioni" non funzionano con Alipay perchè il QR inviato dal server non è valido.
    //Hanno ragione. Il problema però è lato loro in quanto il server cinese crasha se si mandano certi nomi selezione, anche se la codifa del nome in utf8
    //è corretta. Per maggiori dettagli vedi #1554 su redmine.
    //Comunque, la soluzione al momento è di non mandare più il nome della selezione in cinese, ma mandare una stringa ASCII del tipo SELxx in modo che il server
    //cinese non crashi mai
    //
    //sprintf_s (s, sizeof(s), "%s|%d|%s", selectionName, selectionNum, selectionPrice);
    sprintf_s (s, sizeof(s), "SEL%02d|%d|%s", selectionNum, selectionNum, selectionPrice);

	const u32 nBytesToSend = priv_buildCommand ("E13", (const u8*)s, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);
	return true;
}

//***************************************************
void Core::priv_orderAskForPaymentStatus (OSSocket &sok)
{
	u8 buffer[128];
	const u32 nBytesToSend = priv_buildCommand ("E14", (const u8*)theOrder.orderNumber, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);
}

//***************************************************
void Core::priv_orderClose (OSSocket &sok, eOrderCloseStatus statusForServer)
{
	if (theOrder.status == eOrderStatus_none)
		return;
	
	char s[128];
	sprintf_s (s, sizeof(s), "%s|%d", theOrder.orderNumber, (u8)statusForServer);

	u8 buffer[128];
	const u32 nBytesToSend = priv_buildCommand ("E15", (const u8*)s, buffer, sizeof(buffer));
	priv_sendCommand (sok, buffer, nBytesToSend);

	theOrder.status = Core::eOrderStatus_none;

}

//***************************************************
u32 Core::priv_buildCommand (const char *command, const u8 *optionalData, u8 *out_buffer, u32 sizeofOutBuffer) const
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
void Core::priv_sendCommand (OSSocket &sok, const u8 *buffer, u32 nBytesToSend)
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

//***************************************************
void Core::priv_handleIncomingThreadMsg(OSSocket &sok)
{
	rhea::thread::sMsg msg;
	while (rhea::thread::popMsg(hMsgQRead, &msg))
	{
		switch (msg.what)
		{
		case ALIPAYCHINA_SUBSCRIPTION_ANSWER:
			//qualcuno vuole iscriversi alla mia coda di notifiche	
			{
				logger->log("AlipayChinaCore::priv_handleIncomingThreadMsg() => new SUBSCRIPTION_REQUEST\n");
				logger->incIndent();

				//aggiungo la coda alla lista delle code da notificare
				HThreadMsgW hToThreadW;
				hToThreadW.initFromU32 (msg.paramU32);
				toBeNotifiedThreadList.append(hToThreadW);

				//segnalo che ho accettato la sua richiesta
				thread::pushMsg (hToThreadW, ALIPAYCHINA_SUBSCRIPTION_ANSWER_ACCEPTED, (u32)0);

				logger->log("OK");
				logger->decIndent();
			}
			break;

		case ALIPAYCHINA_DIE:
			this->bQuit = true;
			break;

		case ALIPAYCHINA_ASK_ONLINE_STATUS:
			//qualcuno vuole sapere se sono online con il server oppure no
			priv_notify_ONLINE_STATUS_CHANGED();
			break;

		case ALIPAYCHINA_ASK_START_ORDER:
			//qualcuno vuole inziare un ordine
			{
				const u8 *p = (const u8*)msg.buffer;
				const u8 selectionNum = p[0];
				const u8 *selectionName = &p[2];
				const char *selectionPrice = (const char*)&p[2 + p[1] +1];
				if (!priv_orderStart (sok, selectionName, selectionNum, selectionPrice))
				{
					//ALIPAYCHINA_NOTIFY_START_ORDER_RESULT (false)
				}
				else
				{
					//ALIPAYCHINA_NOTIFY_START_ORDER_RESULT
				}
			}
			break;

		case ALIPAYCHINA_ASK_ABORT_ORDER:
			if (theOrder.status != Core::eOrderStatus_pollingPaymentStatus)
			{
				logger->log ("AlipayChinaCore::ALIPAYCHINA_ASK_ABORT_ORDER() => error, order was in wrong status [%d]\n", (u8)theOrder.status);
			}
			else
			{
				logger->log ("AlipayChinaCore::ALIPAYCHINA_ASK_ABORT_ORDER()\n");
				theOrder.timeoutMSec = 0;
			}
			break;

		case ALIPAYCHINA_ASK_END_ORDER:
			if (theOrder.status != Core::eOrderStatus_waitingBeverageEnd)
			{
				logger->log ("AlipayChinaCore::ALIPAYCHINA_ASK_END_ORDER() => error, order was in wrong status [%d]\n", (u8)theOrder.status);
			}
			else
			{
				logger->log ("AlipayChinaCore::ALIPAYCHINA_ASK_END_ORDER(%d)\n", msg.paramU32);
				if (msg.paramU32 == 1)
					theOrder.status = eOrderStatus_finishedOK;
				else
					theOrder.status = eOrderStatus_finishedKO;
			}
			break;
		}
		rhea::thread::deleteMsg(msg);
	}
}

//***************************************************
void Core::priv_doNotifyAll (u16 what, u32 paramU32, const void *data, u32 sizeOfData)
{
	const u32 n = toBeNotifiedThreadList.getNElem();
	for (u32 i = 0; i < n; i++)
		thread::pushMsg (toBeNotifiedThreadList(i), what, paramU32, data, sizeOfData);
}

//***************************************************
void Core::priv_notify_ONLINE_STATUS_CHANGED()
{
	if (bIsLoggedIntoChinaServer)
		priv_doNotifyAll (ALIPAYCHINA_NOTIFY_ONLINE_STATUS_CHANGED, 1, NULL, 0);
	else
		priv_doNotifyAll (ALIPAYCHINA_NOTIFY_ONLINE_STATUS_CHANGED, 0, NULL, 0);
}
//***************************************************
void Core::priv_notify_ORDER_STATUS (AlipayChina::eOrderStatus s, const void *data, u32 sizeOfData)
{
	priv_doNotifyAll (ALIPAYCHINA_NOTIFY_ORDER_STATUS, s, data, sizeOfData);
}
