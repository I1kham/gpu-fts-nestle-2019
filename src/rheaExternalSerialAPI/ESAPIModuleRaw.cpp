#include "ESAPIModuleRaw.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace esapi;

//********************************************************
ModuleRaw::ModuleRaw()
{
    bIsSubscribedTpCPUBridge = false;
}

//*********************************************************
bool ModuleRaw::priv_subscribeToCPUBridge(const HThreadMsgW &hCPUServiceChannelW)
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
                glob->logger->log ("esapi::ModuleRaw => subsribed to CPUBridge\n");

                u8 cpuBridgeVersion = 0;
                cpubridge::translate_SUBSCRIPTION_ANSWER(msg, &cpuBridgeSubscriber, &cpuBridgeVersion);
                rhea::thread::deleteMsg(msg);

                OSEvent h;
                rhea::thread::getMsgQEvent(cpuBridgeSubscriber.hFromMeToSubscriberR, &h);
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


//********************************************************
bool ModuleRaw::setup ( sGlob *glob)
{
    this->glob = glob;

    //buffer
	rs232BufferIN.alloc (glob->localAllocator, 1024);
	rs232BufferOUT = (u8*)RHEAALLOC(glob->localAllocator, SIZE_OF_RS232BUFFEROUT);
    
	//subscription to CPUBridge
    glob->logger->log ("subscribing to cpubridge...");
    bIsSubscribedTpCPUBridge = priv_subscribeToCPUBridge(glob->hCPUServiceChannelW);
    if (bIsSubscribedTpCPUBridge)
        glob->logger->log ("OK\n");
    else
        glob->logger->log ("KO!\n");

    //aggiungo la serviceMsgQ alla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.addEvent(h, WAITLIST_EVENT_FROM_SERVICE_MSGQ);
    }
	return true;
}

//********************************************************
void ModuleRaw::priv_unsetup()
{
    if (bIsSubscribedTpCPUBridge)
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(cpuBridgeSubscriber.hFromMeToSubscriberR, &h);
        waitableGrp.removeEvent (h);
        cpubridge::unsubscribe(cpuBridgeSubscriber);
    }

    //rimuovo la serviceMsgQ dalla waitList
    {
        OSEvent h;
        rhea::thread::getMsgQEvent(glob->serviceMsgQR, &h);
        waitableGrp.removeEvent(h);
    }

#ifdef LINUX
    waitableGrp.removeSerialPort (glob->com);
#endif

    rs232BufferIN.free (glob->localAllocator);
    RHEAFREE(glob->localAllocator, rs232BufferOUT);
}

//********************************************************
eExternalModuleType ModuleRaw::run()
{
#ifdef LINUX
    waitableGrp.addSerialPort (glob->com, WAITLIST_RS232);
#endif

    runningSel.status = cpubridge::eRunningSelStatus_finished_OK;
    retCode = esapi::eExternalModuleType_none;
    while (retCode == esapi::eExternalModuleType_none)
    {
        //qui sarebbe bello rimanere in wait per sempre fino a che un evento non scatta oppure la seriale ha dei dati in input.
        //TODO: trovare un modo multipiattaforma per rimanere in wait sulla seriale (in linux è facile, è windows che rogna un po')
#ifdef LINUX
        const u8 nEvents = waitableGrp.wait(10000);
#else
        const u8 nEvents = waitableGrp.wait(100);
#endif
		for (u8 i = 0; i < nEvents; i++)
		{
			switch (waitableGrp.getEventOrigin(i))
			{
			case OSWaitableGrp::evt_origin_osevent:
				{
                    switch (waitableGrp.getEventUserParamAsU32(i))
                    {
                    case WAITLIST_EVENT_FROM_SERVICE_MSGQ:
                        priv_handleMsgFromServiceQ();
                        break;

                    case WAITLIST_EVENT_FROM_A_SUBSCRIBER:
				        //evento generato dalla msgQ di uno dei miei subscriber
                        {
                            OSEvent h = waitableGrp.getEventSrcAsOSEvent(i);
                            sSubscription *sub = glob->subscriberList.findByOSEvent(h);
						    if (sub)
                                priv_handleMsgFromSubscriber(sub);
				        }
                        break;

                    case WAITLIST_EVENT_FROM_CPUBRIDGE:
                        priv_handleIncomingMsgFromCPUBridge();
                        break;

                    default:
						DBGBREAK;
                        glob->logger->log("esapi::ModuleRaw::run() => unhandled OSEVENT from waitGrp [%d]\n", waitableGrp.getEventUserParamAsU32(i));
					}
				}
				break;

#ifdef LINUX
            case OSWaitableGrp::evt_origin_serialPort:
                priv_rs232_handleCommunication(rs232BufferIN);
                break;
#endif

			default:
                glob->logger->log("esapi::ModuleRaw::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

#ifndef LINUX
        priv_rs232_handleCommunication(rs232BufferIN);
#endif
    }

    priv_unsetup();
    return retCode;
}

//*********************************************************
void ModuleRaw::priv_handleMsgFromServiceQ()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (glob->serviceMsgQR, &msg))
    {
        switch (msg.what)
        {
        default:
            DBGBREAK;
            break;

        case ESAPI_SERVICECH_SUBSCRIPTION_REQUEST:
            {
                sSubscription *sub = glob->subscriberList.onSubscriptionRequest (glob->localAllocator, glob->logger, msg);
                waitableGrp.addEvent(sub->hEvent, WAITLIST_EVENT_FROM_A_SUBSCRIBER);
            }
			break;
		}
		
		rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void ModuleRaw::priv_handleMsgFromSubscriber(sSubscription *sub)
{
    rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
    {
        const u16 handlerID = (u16)msg.paramU32;

        switch (msg.what)
        {
        default:
            glob->logger->log("ModuleRaw::priv_handleMsgFromSubscriber() => invalid msg.what [0x%02X]\n", msg.what);
            break;

        case ESAPI_ASK_UNSUBSCRIBE:
            rhea::thread::deleteMsg(msg);
            waitableGrp.removeEvent (sub->hEvent);
            glob->subscriberList.unsubscribe (glob->localAllocator, sub);
            return;

        case ESAPI_ASK_GET_MODULE_TYPE_AND_VER:
            notify_MODULE_TYPE_AND_VER (sub->q, handlerID, glob->logger, eExternalModuleType_none, this->API_VERSION_MAJOR, this->API_VERSION_MINOR);
            break;
        }

        rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void ModuleRaw::priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg)
{
    cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS(msg, &runningSel.status);
}

//*********************************************************
void ModuleRaw::priv_handleIncomingMsgFromCPUBridge()
{
	rhea::thread::sMsg msg;
    while (rhea::thread::popMsg (cpuBridgeSubscriber.hFromMeToSubscriberR, &msg))
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
                    priv_rs232_sendBuffer (rs232BufferOUT, n);
                }
                break;


            case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
                //risposta al comando # C 2
                {
	                cpubridge::sCPUSelAvailability selAvail;
	                cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msg, &selAvail);

                    const u32 n = esapi::buildMsg_C2_getSelAvailability_resp (selAvail, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
                    priv_rs232_sendBuffer (rs232BufferOUT, n);
                }
                break;

            case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
                priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
                break;

			case CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED:
				//risposta al comando  #C3	
				{
					u8 numPrices = 0;
					u8 numDecimals = 0;
					u16 prices[NUM_MAX_SELECTIONS];
					cpubridge::translateNotify_CPU_SEL_PRICES_CHANGED(msg, &numPrices, &numDecimals, prices);

					u32 BYTES_TO_ALLOC = 5 + numPrices * (7);
					u8 *answer = (u8*)RHEAALLOC(rhea::getScrapAllocator(), BYTES_TO_ALLOC);
					u32 ct = 0;
					answer[ct++] = '#';
					answer[ct++] = 'C';
					answer[ct++] = '3';
					answer[ct++] = numPrices;

					for (u16 i = 0; i < numPrices; i++)
					{
						char s[32];
						const char DECIMAL_SEP = '.';
						rhea::string::format::currency (prices[i], numDecimals, DECIMAL_SEP, s, sizeof(s));

						const u32 n = strlen(s);
						memcpy (&answer[ct], s, n);
						ct += n;
						answer[ct++] = '|';
					}
					answer[ct] = rhea::utils::simpleChecksum8_calc (answer, ct);
					ct++;
					
					priv_rs232_sendBuffer(answer, ct);
					RHEAFREE(rhea::getScrapAllocator(), answer);
				}
				break;
            } //switch (msg.what)
		}
		
		
		rhea::thread::deleteMsg(msg);
    }
}

//*********************************************************
void ModuleRaw::priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend)
{
    rhea::rs232::writeBuffer (glob->com, buffer, numBytesToSend);
}

//*********************************************************
void ModuleRaw::priv_rs232_handleCommunication (sBuffer &b)
{
    const u64 timeToExitMSec = rhea::getTimeNowMSec() + 300;
    while (rhea::getTimeNowMSec() < timeToExitMSec)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);

#ifdef _DEBUG
		if (0 == nBytesAvailInBuffer)
        {
			DBGBREAK;
        }
#endif

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(glob->com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
		    b.numBytesInBuffer += (u16)nRead;
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
            glob->logger->log ("invalid command char [%c]\n", commandChar);
            b.removeFirstNBytes(1);
            break;

        case 'A':   if (!priv_rs232_handleCommand_A (b)) return;    break;
        case 'C':   if (!priv_rs232_handleCommand_C (b)) return;    break;
		case 'R':   if (!priv_rs232_handleCommand_R (b)) return;    break;
		case 'S':   if (!priv_rs232_handleCommand_S (b)) return;    break;
			break;

        }

    } //while(1)

}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool ModuleRaw::priv_rs232_handleCommand_A (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'A';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        glob->logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
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
            priv_rs232_sendBuffer (rs232BufferOUT, n);

            return true;
        }
        break;
    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool ModuleRaw::priv_rs232_handleCommand_C (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'C';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        glob->logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
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

	case '3':
		//Ho ricevuto un ask C3 Get selections prices
		{
			//parse del messaggio
			bool bValidCk = false;
			const u32 MSG_LEN = esapi::buildMsg_C3_getSelAvailability_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk);
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
			cpubridge::ask_CPU_QUERY_SEL_PRICES (this->cpuBridgeSubscriber, 0x01);
			return true;
		}
		break;

    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool ModuleRaw::priv_rs232_handleCommand_S (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'S';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        glob->logger->log ("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
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
			if (selNumber > 0)
			{
				runningSel.status = cpubridge::eRunningSelStatus_wait;
				cpubridge::ask_CPU_START_SELECTION(this->cpuBridgeSubscriber, selNumber);
			}

            //rispondo via seriale confermando di aver ricevuto il msg
            const u32 n = buildMsg_S1_startSelection_resp (selNumber, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (rs232BufferOUT, n);

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
            priv_rs232_sendBuffer (rs232BufferOUT, n);
            return true;
        }
        break;

	case '3':
		//ho recevuto #S3 "start already paid selection"
		{
			//parse del messaggio
			bool bValidCk = false;
			u8 selNum = 0;
			u16 price = 0;
			const u32 MSG_LEN = esapi::buildMsg_S3_startAlreadySelection_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk, &selNum, &price);
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
			if (selNum > 0)
			{
				runningSel.status = cpubridge::eRunningSelStatus_wait;
				cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (this->cpuBridgeSubscriber, selNum, price, cpubridge::eGPUPaymentType_unknown);
			}

			//rispondo via seriale confermando di aver ricevuto il msg
			const u32 n = buildMsg_S3_startAlreadySelection_resp (selNum, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, n);
		}
		break;

	case '4':
		//Ho ricevuto un ask S4, button press
		{
			//parse del messaggio
			bool bValidCk = false;
			u8 btnNum;
			const u32 MSG_LEN = esapi::buildMsg_S4_btnPress_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk, &btnNum);
			if (0 == MSG_LEN)
				return false;
			if (!bValidCk)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			//rimuovo il msg dal buffer
			b.removeFirstNBytes(MSG_LEN);

			//inoltro a CPUBridge
			if (btnNum >0 && btnNum<=12)
				cpubridge::ask_CPU_SEND_BUTTON (this->cpuBridgeSubscriber, btnNum);

			//rispondo
			const u32 n = esapi::buildMsg_S4_btnPress_resp (btnNum, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
			priv_rs232_sendBuffer (rs232BufferOUT, n);
			return true;
		}
		break;
    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool ModuleRaw::priv_rs232_handleCommand_R (sBuffer &b)
{
	const u8 COMMAND_CHAR = 'R';

    assert(b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
	const u8 commandCode = b.buffer[2];

	switch (commandCode)
	{
	default:
		glob->logger->log("invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		b.removeFirstNBytes(2);
		return true;

	case '1':
		//Ho ricevuto un ask R1 External module identify
        //C'è un modulo esterno, collegato alla seriale che vuole identificarsi
		{
            //parse del messaggio
            bool bValidCk = false;
            const u32 MSG_LEN = esapi::buildMsg_R1_externalModuleIdentify_parseAsk (b.buffer, b.numBytesInBuffer, &bValidCk, &glob->moduleInfo.type, &glob->moduleInfo.verMajor, &glob->moduleInfo.verMinor);
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
			switch (glob->moduleInfo.type)
			{
			case eExternalModuleType_rasPI_wifi_REST:
				result = 0x01;
                retCode = glob->moduleInfo.type;
				break;

			default:
				result = 0x00;
				break;
			}

            const u32 n = esapi::buildMsg_R1_externalModuleIdentify_resp (result, rs232BufferOUT, SIZE_OF_RS232BUFFEROUT);
            priv_rs232_sendBuffer (rs232BufferOUT, n);
			return true;
		}
		break;
	}
}
