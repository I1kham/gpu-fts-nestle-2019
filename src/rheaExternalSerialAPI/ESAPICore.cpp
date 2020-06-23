#include "ESAPICore.h"
#include "ESAPI.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaBit.h"
#include "../rheaCommonLib/rheaUtils.h"


using namespace esapi;

//*********************************************************
Core::Core()
{
	localAllocator = NULL;
	logger = &nullLogger;
	rhea::rs232::setInvalid (com);
    rs232BufferOUT = NULL;
}

//*********************************************************
bool Core::priv_subscribeToCPUBridge()
{
    //creo una msgQ temporanea per ricevere da CPUBridge la risposta alla mia richiesta di iscrizione
    HThreadMsgR hTempMsgQR;
    HThreadMsgW hTempMsgQW;
    rhea::thread::createMsgQ (&hTempMsgQR, &hTempMsgQW);

    //mando la richiesta
    cpubridge::subscribe (hCPUServiceChannelW, hTempMsgQW);

    //attendo risposta
    bool bSubscribed = false;
    u64 timeToExitMSec = rhea::getTimeNowMSec() + 2000;
    do
    {
        rhea::thread::sleepMSec(50);

        rhea::thread::sMsg msg;
        if (rhea::thread::popMsg(hTempMsgQR, &msg))
        {
            //ok, ci siamo
            if (msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER)
            {
                logger->log ("esapi::Core => subsribed to CPUBridge\n");

                u8 cpuBridgeVersion = 0;
                cpubridge::translate_SUBSCRIPTION_ANSWER(msg, &cpuBridgeSubscriber, &cpuBridgeVersion);
                rhea::thread::deleteMsg(msg);

                OSEvent h;
                rhea::thread::getMsgQEvent(cpuBridgeSubscriber.hFromCpuToOtherR, &h);
                waitableGrp.addEvent (h, WAITLIST_EVENT_FROM_CPUBRIDGE);
                bSubscribed = true;
                break;
            }

            rhea::thread::deleteMsg(msg);
        }
    } while (rhea::getTimeNowMSec() < timeToExitMSec);

    //delete della msgQ
    rhea::thread::deleteMsgQ (hTempMsgQR, hTempMsgQW);
    return bSubscribed;
}

//*******************************************************
Core::sConnectedSocket* Core::priv_2280_findConnectedSocketByUID (u32 uid)
{
	for (u32 i = 0; i < sockettList.getNElem(); i++)
	{
		if (sockettList(i).uid == uid)
			return &sockettList[i];
	}
	return NULL;
}

//*********************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
    if (NULL==loggerIN)
        logger = &nullLogger;
    else
        logger = loggerIN;
}

//*********************************************************
bool Core::open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW)
{
	const bool SERIAL_IS_BLOCKING = false;

    this->hCPUServiceChannelW = hCPUServiceChannelW;

    logger->log ("esapi::Core::open\n");
    logger->incIndent();

	logger->log ("com=%s   ", serialPort);
    if (!rhea::rs232::open(&com, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        logger->log ("FAILED. unable to open port [%s]\n", serialPort);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

    localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("mitm");

    //buffer vari
    serialBuffer.alloc (localAllocator, 2048);
    rs232BufferOUT = (u8*)RHEAALLOC(localAllocator, SIZE_OF_RS232BUFFEROUT);
	sokBuffer = (u8*)RHEAALLOC(localAllocator, SIZE_OF_SOKBUFFER);
	sockettList.setup (localAllocator, 128);

    logger->log ("subsribing to cpubridge...");
    if (priv_subscribeToCPUBridge())
        logger->log ("OK\n");
    else
        logger->log ("KO!\n");

	logger->log ("OK\n");
	logger->decIndent();
    return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (com);

    if (localAllocator)
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(cpuBridgeSubscriber.hFromCpuToOtherR, &h);
        waitableGrp.removeEvent (h);

		sockettList.unsetup();
		serialBuffer.free (localAllocator);
        RHEAFREE(localAllocator, rs232BufferOUT);
		RHEAFREE(localAllocator, sokBuffer);
        RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
        localAllocator = NULL;
    }
}

//*********************************************************
void Core::run()
{
    runningSel.status = cpubridge::eRunningSelStatus_finished_OK;
    bQuit = false;
    while (bQuit == false)
    {
		//qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
		//TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
        const u8 nEvents = waitableGrp.wait(10);
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case WAITLIST_EVENT_FROM_CPUBRIDGE:
                        priv_handleIncomingMsgFromCPUBridge();
                        break;
                    
                    default:
						DBGBREAK;
                        logger->log("esapi::Core::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

			case OSWaitableGrp::evt_origin_socket:
				{
					//Ho ricevuto dei dati lungo la socket, devo spedirli via seriale al rasPI
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
					OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
					priv_2280_sendDataViaRS232 (sok, clientUID);
				}
				break;

			default:
                logger->log("esapi::Core::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

        //gestione comunicazione seriale
        priv_rs232_handleCommunication(com, serialBuffer);
    }
}

//*********************************************************
void Core::priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg)
{
    cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS(msg, &runningSel.status);
}


//*********************************************************
void Core::priv_handleIncomingMsgFromCPUBridge()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (cpuBridgeSubscriber.hFromCpuToOtherR, &msg))
    {
        const u16 handlerID = (msg.paramU32 & 0x0000FFFF);

        if (handlerID == 0)
        {
            //in queso caso, CPUBridge ha mandato una notifica di sua spontanea volontà , non è una risposta ad una mia specifica richiesta.
            switch (msg.what)
            {
            case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
                priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
                break;
            }
        }
        else
        {
            //in questo caso invece, è una risposta ad una mia domanda specifica
            switch (msg.what)
            {
            default:
                DBGBREAK;
                break;

            case CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE:
                //risposta al comando # C 1
                {
	                cpubridge::sCPULCDMessage lcdMsg;
	                translateNotify_CPU_NEW_LCD_MESSAGE(msg, &lcdMsg);

	                const u16 msgLenInBytes = rhea::string::utf16::lengthInBytes(lcdMsg.utf16LCDString);
                    const u32 n = esapi::buildMsg_C1_getCPUScreenMsg_resp (lcdMsg.utf16LCDString, msgLenInBytes, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
                    priv_rs232_sendBuffer (com, rs232BufferOUT, n);
                }
                break;


            case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
                //risposta al comando # C 2
                {
	                cpubridge::sCPUSelAvailability selAvail;
	                cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msg, &selAvail);

                    const u32 n = esapi::buildMsg_C2_getSelAvailability_resp (selAvail, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
                    priv_rs232_sendBuffer (com, rs232BufferOUT, n);
                }
                break;

            case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
                priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
                break;

            } //switch (msg.what)
		}
		
		
		rhea::thread::deleteMsg(msg);
    }
}



//*********************************************************
void Core::priv_rs232_handleCommunication (OSSerialPort &comPort, Core::sBuffer &b)
{
    while (1)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);

		if (0 == nBytesAvailInBuffer)
			DBGBREAK;

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(comPort, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
		    b.numBytesInBuffer += (u16)nRead;

			if (nRead)
				logger->log ("rs232: [%d] in buffer\n", b.numBytesInBuffer);
	    }
    
		if (0 == b.numBytesInBuffer)
			return;

        //provo ad estrarre un msg 'raw' dal mio buffer.
        //Cerco il carattere di inizio buffer ed eventualmente butto via tutto quello che c'è prima
        u32 i = 0;
        while (i < b.numBytesInBuffer && b.buffer[i] != (u8)'#')
            i++;

        if (b.buffer[i] != (u8)'#')
        {
            b.reset();
            return;
        }

        b.removeFirstNBytes(i);
        assert (b.buffer[0] == (u8)'#');
        i = 0;

        if (b.numBytesInBuffer < 3)
            return;

        const u8 commandChar = b.buffer[1];
        switch (commandChar)
        {
        default:
            logger->log ("invalid command char [%c]\n", commandChar);
            b.removeFirstNBytes(1);
            break;

        case 'A':   if (!priv_rs232_handleCommand_A (comPort, b)) return;    break;
        case 'C':   if (!priv_rs232_handleCommand_C (comPort, b)) return;    break;
		case 'R':   if (!priv_rs232_handleCommand_R (comPort, b)) return;    break;
		case 'S':   if (!priv_rs232_handleCommand_S (comPort, b)) return;    break;
			break;

        }

    } //while(1)

}

//*********************************************************
void Core::priv_rs232_sendBuffer (OSSerialPort &comPort, const u8 *buffer, u32 numBytesToSend)
{
    rhea::rs232::writeBuffer (comPort, buffer, numBytesToSend);
}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_rs232_handleCommand_A (OSSerialPort &comPort, sBuffer &b)
{
    const u8 COMMAND_CHAR = 'A';

    assert (b.numBytesInBuffer >= 3 & b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];
    bool ret;

    switch (commandCode)
    {
    default:
        logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //ho ricevuto un ask A1 Api version
        {
           //parse del messaggio
            bool bValidCk = false;
            const u32 MSG_LEN = esapi::buildMsg_A1_getAPIVersion_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }
        
            //rimuovo msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

            //rispondo
            const u32 n = esapi::buildMsg_A1_getAPIVersion_resp (API_VERSION_MAJOR, API_VERSION_MINOR, esapi::eGPUType_TS, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (comPort, rs232BufferOUT, n);

            return true;
        }
        break;
    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Core::priv_rs232_handleCommand_C (OSSerialPort &comPort, sBuffer &b)
{
    const u8 COMMAND_CHAR = 'C';

    assert (b.numBytesInBuffer >= 3 & b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];
    bool ret;

    switch (commandCode)
    {
    default:
        logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //Ho ricevuto un ask C1 query LCD message
        {
           //parse del messaggio
            bool bValidCk = false;
            const u32 MSG_LEN = esapi::buildMsg_C1_getCPUScreenMsg_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }
        
            //rimuovo msg dal buffer
            b.removeFirstNBytes(MSG_LEN);
        
            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi priv_handleIncomingMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_LCD_MESSAGE (this->cpuBridgeSubscriber, 0x01);
            return true;
        }
        break;

    case '2': 
        //Ho ricevuto un ask C2 Get selections availability
        {
           //parse del messaggio
            bool bValidCk = false;
            const u32 MSG_LEN = esapi::buildMsg_C2_getSelAvailability_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }
        
            //rimuovo msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi priv_handleIncomingMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_SEL_AVAIL (this->cpuBridgeSubscriber, 0x01);
            return true;
        }
        break;

    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Core::priv_rs232_handleCommand_S (OSSerialPort &comPort, sBuffer &b)
{
    const u8 COMMAND_CHAR = 'S';

    assert (b.numBytesInBuffer >= 3 & b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];
    bool ret;

    switch (commandCode)
    {
    default:
        logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //Ho ricevuto un ask S1 Start selection
        {
            //parse del messaggio
            bool bValidCk = false;
            u8 selNumber = 0;
            const u32 MSG_LEN = esapi::buildMsg_S1_startSelection_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk, &selNumber);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

            //chiedo a CPUBridge di iniziare la selezione indicata. CPUBrdige risponderà con una serie di notify_CPU_RUNNING_SEL_STATUS() per
            //indicare lo stato di avanzamento della selezione
            runningSel.status = cpubridge::eRunningSelStatus_wait;
            const u16 price = 0xffff; //serve per fare in modo che la CPU gestisca lei il pagamento
            cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (this->cpuBridgeSubscriber, selNumber, price, cpubridge::eGPUPaymentType_unknown);

            //rispondo via seriale confermando di aver ricevuto il msg
            const u32 n = buildMsg_S1_startSelection_resp (selNumber, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (comPort, rs232BufferOUT, n);

            return true;
        }
        break;

    case '2': 
        //Ho ricevuto un ask S2 query selection status
        {
            //parse del messaggio
            bool bValidCk = false;
            const u32 MSG_LEN = esapi::buildMsg_S2_querySelectionStatus_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

            //rispondo
            const u32 n = esapi::buildMsg_S2_querySelectionStatus_resp (runningSel.status, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (comPort, rs232BufferOUT, n);
            return true;
        }
        break;

    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Core::priv_rs232_handleCommand_R (OSSerialPort &comPort, sBuffer &b)
{
	const u8 COMMAND_CHAR = 'R';

	assert(b.numBytesInBuffer >= 3 & b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
	const u8 commandCode = b.buffer[2];
	bool ret;

	switch (commandCode)
	{
	default:
		logger->log("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		b.removeFirstNBytes(2);
		return true;

	case '1':
		//Ho ricevuto un ask R1 External module identify
        //C'è un modulo esterno, collegato alla seriale che vuole identificarsi
		{
            //parse del messaggio
            bool bValidCk = false;
            eExternalModuleType moduleType;
            u8 moduleVerMajor;
            u8 moduleVerMinor;
            const u32 MSG_LEN = esapi::buildMsg_R1_externalModuleIdentify_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk, &moduleType, &moduleVerMajor, &moduleVerMinor);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

           //rispondo via seriale confermando di aver ricevuto il msg
            u8 result = 0x00;
			switch (moduleType)
			{
			case eExternalModuleType_rasPI_wifi_REST:
				result = 0x01;
				break;

			default:
				result = 0x00;
				break;
			}

            const u32 n = esapi::buildMsg_R1_externalModuleIdentify_resp (result, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (comPort, rs232BufferOUT, n);
			return true;
		}
		break;

	case 0x01:
		//ho ricevuto un R 0x01 new socket connected
		{
            //parse del messaggio
            bool bValidCk = false;
            u32 socketUID = 0;
            const u32 MSG_LEN = esapi::buildMsg_R0x01_newSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//creo una nuova socket e la metto in comunicazione con sokbridge
			sConnectedSocket cl;
			rhea::socket::init (&cl.sok);
			logger->log ("new socket connection...");
			eSocketError err = rhea::socket::openAsTCPClient (&cl.sok, "127.0.0.1", 2280);
			if (err != eSocketError_none)
			{
				logger->log ("FAIL\n");
				DBGBREAK;
				//comunico la disconnessione via seriale
                const u32 n = esapi::buildMsg_R0x02_closeSocket (socketUID, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			    priv_rs232_sendBuffer (com, rs232BufferOUT, n);
			}
			else
			{
				cl.uid = socketUID;
				sockettList.append(cl);
				waitableGrp.addSocket (cl.sok, cl.uid);
				logger->log ("OK, socket id [%d]\n", cl.uid);
			}
			return true;
		}
		break;

	case 0x02:
		//ho ricevuto un R 0x02 socket close
		{
            //parse del messaggio
            bool bValidCk = false;
            u32 socketUID = 0;
            const u32 MSG_LEN = esapi::buildMsg_R0x02_closeSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }

            //rimuovo il msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findConnectedSocketByUID (socketUID);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, socketUID);
		}
		return true;

	case 0x03:
		//rasPIESAPI: rasPI comunica via seriale che la socket [client_uid_4bytes] ha ricevuto i dati  [data] per un totale di [lenMSB][lenLSB] bytes
		//rcv:	# R [0x03] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data…] [ck]
		{
			if (b.numBytesInBuffer < 9)
				return false;

			const u32 uid = rhea::utils::bufferReadU32 (&b.buffer[3]);
			const u16 dataLen = rhea::utils::bufferReadU16(&b.buffer[7]);

			if (b.numBytesInBuffer < 10 + dataLen)
				return false;

			const u8* data = &b.buffer[9];
			const u8 ck = b.buffer[9 + dataLen];
			if (rhea::utils::simpleChecksum8_calc(b.buffer, 9 + dataLen) != ck)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			//messaggio valido, lo devo mandare via socket al client giusto
			if (dataLen)
			{
				sConnectedSocket *cl = priv_2280_findConnectedSocketByUID(uid);
				if (NULL != cl)
				{
					rhea::socket::write (cl->sok, data, dataLen);
					logger->log ("rcv [%d] bytes from RS232, sending to socket [%d]\n", dataLen, cl->uid);
				}
				else
				{
					DBGBREAK;
				}
			}

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(10 + dataLen);
			return true;
		}
		break;
	}
}

//***************************************************************
void Core::priv_2280_onClientDisconnected (OSSocket &sok, u32 uid)
{
	for (u32 i = 0; i < sockettList.getNElem(); i++)
	{
		if (sockettList(i).uid == uid)
		{
			assert (rhea::socket::compare(sockettList(i).sok, sok));

			waitableGrp.removeSocket (sockettList[i].sok);
			rhea::socket::close(sockettList[i].sok);
			sockettList.removeAndSwapWithLast(i);

			//comunico la disconnessione via seriale
            const u32 n = esapi::buildMsg_R0x02_closeSocket (uid, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (com, rs232BufferOUT, n);
			logger->log ("socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;
}

//***************************************************************
void Core::priv_2280_sendDataViaRS232 (OSSocket &sok, u32 uid)
{
	sConnectedSocket *cl = priv_2280_findConnectedSocketByUID(uid);
	if (NULL == cl)
	{
		DBGBREAK;
		return;
	}
	assert (rhea::socket::compare(cl->sok, sok));


	i32 nBytesLetti = rhea::socket::read (cl->sok, sokBuffer, SIZE_OF_SOKBUFFER, 100);
	if (nBytesLetti == 0)
	{
		//connessione chiusa
		priv_2280_onClientDisconnected(sok, uid);
		return;
	}
	if (nBytesLetti < 0)
	{
		//la chiamata sarebbe stata bloccante, non dovrebbe succedere
		logger->log ("WARN: socket [%d], read bloccante...\n", cl->uid);
		return;
	}

	//spedisco lungo la seriale
	u32 ct = 0;
	rs232BufferOUT[ct++] = '#';
	rs232BufferOUT[ct++] = 'R';
	rs232BufferOUT[ct++] = 0x04;

	rhea::utils::bufferWriteU32 (&rs232BufferOUT[ct], cl->uid);
	ct += 4;

	rhea::utils::bufferWriteU16 (&rs232BufferOUT[ct], nBytesLetti);
	ct += 2;

	memcpy (&rs232BufferOUT[ct], sokBuffer, nBytesLetti);
	ct += nBytesLetti;

	rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
	ct++;

	priv_rs232_sendBuffer (com, rs232BufferOUT, ct);
	logger->log ("rcv [%d] bytes from socket [%d], sending to rasPI\n", nBytesLetti, cl->uid);
}