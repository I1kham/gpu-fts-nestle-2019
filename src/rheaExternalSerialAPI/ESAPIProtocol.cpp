#include "ESAPIProtocol.h"
#include "ESAPI.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace esapi;

//*********************************************************
Protocol::Protocol()
{
    rhea::rs232::setInvalid (rs232);
    localAllocator = NULL;
    logger = NULL;
    cpuBridgeSubscriber = NULL;
    waitableGrp = NULL;
    bufferOUT = NULL;
    runningSel.status = cpubridge::eRunningSelStatus_finished_OK;
}

//*********************************************************
bool Protocol::setup (rhea::Allocator *allocator, cpubridge::sSubscriber *cpuBridgeSubscriber, const char *serialPort, OSWaitableGrp *waitableGrp, rhea::ISimpleLogger *loggerIN)
{
    this->localAllocator = allocator;
    this->logger = loggerIN;
    this->cpuBridgeSubscriber = cpuBridgeSubscriber;

    //apertura rs232
	logger->log ("esapi::Protocol::setup => opening com=%s   ", serialPort);
    const bool SERIAL_IS_BLOCKING = false;
    if (!rhea::rs232::open(&rs232, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, SERIAL_IS_BLOCKING))
    {
        logger->log ("FAILED. unable to open port [%s]\n", serialPort);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

#ifdef LINUX
    waitableGrp->addSerialPort (glob->com, ESAPI_WAITABLEGRP_EVENT_FROM_RS232);
#endif

    //buffer
    bufferOUT = RHEAALLOCT(u8*, localAllocator, SIZE_OF_BUFFEROUT);
    rs232BufferIN.alloc (localAllocator, 1024);

    return true;
}

//*********************************************************
void Protocol::priv_unsetup()
{
    if (NULL == localAllocator)
        return;

    if (NULL != waitableGrp)
    {
#ifdef LINUX
        waitableGrp.removeSerialPort (glob->com);
#endif
        waitableGrp = NULL;
    }

    rhea::rs232::close (rs232);

    if (bufferOUT)
    {
        RHEAFREE(localAllocator, bufferOUT);
        bufferOUT = NULL;
    }
    rs232BufferIN.free(localAllocator);

    localAllocator = NULL;
}

//*********************************************************
void Protocol::priv_onCPUNotify_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg)
{
    cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS(msg, &runningSel.status);
}

//*********************************************************
bool Protocol::onMsgFromCPUBridge(cpubridge::sSubscriber &cpuBridgeSubscriber, const rhea::thread::sMsg &msg, u16 handlerID)
{
    if (handlerID == 0)
    {
        //in queso caso, CPUBridge ha mandato una notifica di sua spontanea volontà , non è una risposta ad una mia specifica richiesta.
        switch (msg.what)
        {
        default:
            return false;

        case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
            priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
            return true;
		}
    }
    else
    {
        //in questo caso invece, è una risposta ad una mia domanda specifica
        switch (msg.what)
        {
        default:
            return false;

        case CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE:
            //risposta al comando # C 1
            {
	            cpubridge::sCPULCDMessage lcdMsg;
	            translateNotify_CPU_NEW_LCD_MESSAGE(msg, &lcdMsg);

	            const u16 msgLenInBytes = rhea::string::utf16::lengthInBytes(lcdMsg.utf16LCDString);
                const u32 n = esapi::buildMsg_C1_getCPUScreenMsg_resp (lcdMsg.utf16LCDString, msgLenInBytes, bufferOUT, SIZE_OF_BUFFEROUT);
                rs232_write (bufferOUT, n);
            }
            return true;

        case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
            //risposta al comando # C 2
            {
	            cpubridge::sCPUSelAvailability selAvail;
	            cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msg, &selAvail);

                const u32 n = esapi::buildMsg_C2_getSelAvailability_resp (selAvail, bufferOUT, SIZE_OF_BUFFEROUT);
                rs232_write (bufferOUT, n);
            }
            return true;

        case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
            priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
            return true;

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
					
				rs232_write (answer, ct);
				RHEAFREE(rhea::getScrapAllocator(), answer);
			}
			return true;
        } //switch (msg.what)
	}

    //qui non ci dovremmo mai arrivare
    DBGBREAK;
    return false;
}


//*********************************************************
void Protocol::rs232_write (const u8 *buffer, u32 numBytesToSend)
{
    if (numBytesToSend>0)
        rhea::rs232::writeBuffer (rs232, buffer, numBytesToSend);
}

//*********************************************************
bool Protocol::rsr232_readSingleByte (u8 *out)
{
    return rhea::rs232::readBuffer(rs232, out, 1);
}

//*********************************************************
bool Protocol::rs232_read ()
{
    const u64 timeToExitMSec = rhea::getTimeNowMSec() + 300;
    while (rhea::getTimeNowMSec() < timeToExitMSec)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(rs232BufferIN.SIZE - rs232BufferIN.numBytesInBuffer);

#ifdef _DEBUG
		if (0 == nBytesAvailInBuffer)
        {
			DBGBREAK;
        }
#endif

	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(rs232, &rs232BufferIN.buffer[rs232BufferIN.numBytesInBuffer], nBytesAvailInBuffer);
		    rs232BufferIN.numBytesInBuffer += (u16)nRead;
	    }
    
		if (0 == rs232BufferIN.numBytesInBuffer)
			return true;

        //provo ad estrarre un msg 'raw' dal mio buffer.
        //Cerco il carattere di inizio buffer ed eventualmente butto via tutto quello che c'è prima
        u32 i = 0;
        while (i < rs232BufferIN.numBytesInBuffer && rs232BufferIN.buffer[i] != (u8)'#')
            i++;

        if (rs232BufferIN.buffer[i] != (u8)'#')
        {
            rs232BufferIN.reset();
            return true;
        }

        rs232BufferIN.removeFirstNBytes(i);
        assert (rs232BufferIN.buffer[0] == (u8)'#');
        i = 0;

        if (rs232BufferIN.numBytesInBuffer < 3)
            return true;

        const u8 commandChar = rs232BufferIN.buffer[1];
        switch (commandChar)
        {
        default:
            logger->log ("esapi::Protocol() => invalid command char [%c]\n", commandChar);
            rs232BufferIN.removeFirstNBytes(1);
            break;

        case 'A':   if (!priv_rs232_handleCommand_A (rs232BufferIN)) return true;    break;
        case 'C':   if (!priv_rs232_handleCommand_C (rs232BufferIN)) return true;    break;
		case 'S':   if (!priv_rs232_handleCommand_S (rs232BufferIN)) return true;    break;

        case 'R':
            //questo tipo di comandi (classe 'R') devono essere gestiti in autonomia dai singoli moduli
            return false;
        }
    }

    return true;
}

/*********************************************************
 *  ritorna true se ha consumato qualche byte di buffer.
 *  ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Protocol::priv_rs232_handleCommand_A (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'A';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol() => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //Request API version
        //ricevuto: # A 1 [ck]
        //rispondo: # A 1 [api_ver_major] [api_ver_minor] [GPUmodel] [ck]
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
            const u32 n = esapi::buildMsg_A1_getAPIVersion_resp (ESAPI_VERSION_MAJOR, ESAPI_VERSION_MINOR, esapi::eGPUType_TS, bufferOUT, SIZE_OF_BUFFEROUT);
            rs232_write (bufferOUT, n);
            return true;
        }
        break;
    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Protocol::priv_rs232_handleCommand_C (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'C';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //Query CPU screen message
        //ricevuto: # C 1 [ck]
        //rispondo: # C 1 [numByteInMsg] [messageUTF16_LSB_MSB] [ck]
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
        
            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_LCD_MESSAGE (*cpuBridgeSubscriber, 0x01);
            return true;
        }
        break;

    case '2': 
        //Query selections availability
        //ricevuto: # C 2 [ck]
        //rispondo: # C 2 [avail1-8] [avail9-16] [avail17-24] [...] [avail121-128] [ck]
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
            cpubridge::ask_CPU_QUERY_SEL_AVAIL (*cpuBridgeSubscriber, 0x01);
            return true;
        }
        break;

	case '3':
        //Query selections prices
        //ricevuto: # C 3 [ck]
        //rispondo: # C 3 [n] [price1] | [price2] | … | [priceN] | [ck]
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
			cpubridge::ask_CPU_QUERY_SEL_PRICES (*cpuBridgeSubscriber, 0x01);
			return true;
		}
		break;

    case '4':
        //Query 12 LED status
        //ricevuto: # C 3 [ck]
        //rispondo: # C 4 [b1] [b2] [ck]
        DBGBREAK;
        logger->log ("esapi::Protocol => #C4 not implemented yet\n");
        b.removeFirstNBytes(2);
        return true;

    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Protocol::priv_rs232_handleCommand_S (sBuffer &b)
{
    const u8 COMMAND_CHAR = 'S';

    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == COMMAND_CHAR);
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
        b.removeFirstNBytes(2);
        return true;

    case '1': 
        //Start selection
        //ricevuto: # S 1 [sel_num] [ck]
        //rispondo: # S 1 [sel_num] [ck]
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

            //chiedo a CPUBridge di iniziare la selezione indicata. CPUBridge risponderà con una serie di notify_CPU_RUNNING_SEL_STATUS() per
            //indicare lo stato di avanzamento della selezione
			if (selNumber > 0)
			{
				runningSel.status = cpubridge::eRunningSelStatus_wait;
				cpubridge::ask_CPU_START_SELECTION(*cpuBridgeSubscriber, selNumber);
			}

            //rispondo via seriale confermando di aver ricevuto il msg
            const u32 n = buildMsg_S1_startSelection_resp (selNumber, bufferOUT, SIZE_OF_BUFFEROUT);
            rs232_write (bufferOUT, n);

            return true;
        }
        break;

    case '2': 
        //Query selection status
        //ricevuto: # S 2 [ck]
        //rispondo: # S 2 [status] [ck]
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
            const u32 n = esapi::buildMsg_S2_querySelectionStatus_resp (runningSel.status, bufferOUT, SIZE_OF_BUFFEROUT);
            rs232_write (bufferOUT, n);
            return true;
        }
        break;

	case '3':
		//start already paid selection
        //ricevuto: # S 3 [sel_num] [priceLSB] [priceMSB] [ck]
        //rispondo: # S 3 [sel_num] [ck]
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
				cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (*cpuBridgeSubscriber, selNum, price, cpubridge::eGPUPaymentType_unknown);
			}

			//rispondo via seriale confermando di aver ricevuto il msg
			const u32 n = buildMsg_S3_startAlreadySelection_resp (selNum, bufferOUT, SIZE_OF_BUFFEROUT);
			rs232_write (bufferOUT, n);
            return true;
		}
		break;

	case '4':
		//Send button press
        //ricevuto: # S 4 [btn] [ck]
        //rispondo: # S 4 [btn] [ck]
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
				cpubridge::ask_CPU_SEND_BUTTON (*cpuBridgeSubscriber, btnNum);

			//rispondo
			const u32 n = esapi::buildMsg_S4_btnPress_resp (btnNum, bufferOUT, SIZE_OF_BUFFEROUT);
			rs232_write (bufferOUT, n);
			return true;
		}
		break;
    }
}

