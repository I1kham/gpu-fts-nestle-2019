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
    runningSel.status = cpubridge::eRunningSelStatus::finished_OK;
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
    if (!rhea::rs232::open(&rs232, serialPort, eRS232BaudRate::b115200, false, false, eRS232DataBits::b8, eRS232Parity::No, eRS232StopBits::One, eRS232FlowControl::No, SERIAL_IS_BLOCKING))
    {
        logger->log ("FAILED. unable to open port [%s]\n", serialPort);
        logger->decIndent();
        return false;
    }
	logger->log ("OK\n");

#ifdef LINUX
    waitableGrp->addSerialPort (rs232, ESAPI_WAITABLEGRP_EVENT_FROM_RS232);
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
        waitableGrp->removeSerialPort (rs232);
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
bool Protocol::onMsgFromCPUBridge(UNUSED_PARAM cpubridge::sSubscriber &cpuBridgeSubscriber, const rhea::thread::sMsg &msg, u16 handlerID)
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
			DBGBREAK;
			return false;
		
		case CPUBRIDGE_NOTIFY_QUERY_ID101:
            //risposta al comando  #A2
            {
	            u32 id101 = 0;
	            cpubridge::translateNotify_CPU_QUERY_ID101(msg, &id101);

				//rispondo: # A 2 'R' 'V' 'H' [id101 come stringa di 9 ASCCI char] [ck]
				char s[16];
				sprintf_s (s, sizeof(s), "RVH%09d", id101);
	            rs232_esapiSendAnswer ('A', '2', s, 12);
            }
            return true;


        case CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE:
            //risposta al comando  #C1
            {
	            cpubridge::sCPULCDMessage lcdMsg;
	            translateNotify_CPU_NEW_LCD_MESSAGE(msg, &lcdMsg);

	            const u16 msgLenInBytes = rhea::string::utf16::lengthInBytes(lcdMsg.utf16LCDString);
                const u32 n = esapi::buildAnswer_C1_getCPUScreenMsg (lcdMsg.utf16LCDString, msgLenInBytes, bufferOUT, SIZE_OF_BUFFEROUT);
                rs232_write (bufferOUT, n);
            }
            return true;

        case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
            //risposta al comando  #C2
            {
	            cpubridge::sCPUSelAvailability selAvail;
	            cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msg, &selAvail);

                const u32 n = esapi::buildAnswer_C2_getSelAvailability (selAvail, bufferOUT, SIZE_OF_BUFFEROUT);
                rs232_write (bufferOUT, n);
            }
            return true;

        case CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS:
            priv_onCPUNotify_RUNNING_SEL_STATUS(msg);
            return true;

        case CPUBRIDGE_NOTIFY_SINGLE_SEL_PRICE_STRING:
			//risposta al comando  #C3	
			{
                u8 data[32];
                u8 selNum = 0;
                cpubridge::translateNotify_CPU_SINGLE_SEL_PRICE (msg, &selNum, &data[2], sizeof(data)-2);

                //rispondo con # C 3 [selNum] [priceLen] [price?] [ck]
                data[0] = selNum;
                data[1] = (u8)rhea::string::utf8::lengthInBytes(&data[2]);

                rs232_esapiSendAnswer ('C', '3', data, data[1]+2);
            }
			return true;

		case CPUBRIDGE_NOTIFY_SELECTION_NAME_UTF16_LSB_MSB:
			//risposta al comando  #C5
			{
				u8 selNum = 0;
				u16 selNameUTF16_LSB_MSB[64];
				cpubridge::translateNotify_CPU_GET_CPU_SELECTION_NAME (msg, &selNum, selNameUTF16_LSB_MSB, sizeof(selNameUTF16_LSB_MSB));

				//rispondo # C 5 [selNum] [nameLenInBytes] [nameUTF16_LSB_MSB...] [ck]
				const u8 nameLenInBytes = rhea::string::utf16::lengthInBytes(selNameUTF16_LSB_MSB);

				u32 BYTES_TO_ALLOC = 6 + nameLenInBytes;
				u8 *answer = (u8*)RHEAALLOC(rhea::getScrapAllocator(), BYTES_TO_ALLOC);
				u16 ct = 0;
				answer[ct++] = '#';
				answer[ct++] = 'C';
				answer[ct++] = '5';
				answer[ct++] = selNum;
				answer[ct++] = nameLenInBytes;
				memcpy (&answer[ct], selNameUTF16_LSB_MSB, nameLenInBytes);
				ct += nameLenInBytes;

				answer[ct] = rhea::utils::simpleChecksum8_calc (answer, ct);
				ct++;

				rs232_write (answer, ct);
				RHEAFREE(rhea::getScrapAllocator(), answer);
			}
			return true;

		case CPUBRIDGE_NOTIFY_CPU_FULLSTATE:
			//risposta al comando  #C6
			{
				cpubridge::sCPUStatus s;
				cpubridge::translateNotify_CPU_FULLSTATE (msg, &s);

				//if (0x0001 == handlerID)
				//{
					u8 status = 'N';
					if (s.isCupDetected())
						status = 'Y';

					//rispondo con # C 6 [status] [ck] 
					rs232_esapiSendAnswer ('C', '6', &status, 1);
					//}
			}
			return true;
        } //switch (msg.what)
	}
}


//*********************************************************
void Protocol::rs232_write (const u8 *buffer, u32 numBytesToSend)
{
    if (numBytesToSend>0)
        rhea::rs232::writeBuffer (rs232, buffer, numBytesToSend);
}

//*********************************************************
void Protocol::rs232_esapiSendAnswer (u8 c1, u8 c2, const void* optionalData, u32 numOfBytesInOptionalData)
{
	const u32 nBytesToSend = esapi::buildAnswer (c1, c2, optionalData, numOfBytesInOptionalData, bufferOUT, SIZE_OF_BUFFEROUT);
	if (nBytesToSend)
		rs232_write (bufferOUT, nBytesToSend);
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

		//il comando valido più piccolo è di almeno 4 byte
        if (rs232BufferIN.numBytesInBuffer < 4)
            return true;

        const u8 commandChar = rs232BufferIN.buffer[1];
		u32 nBytesConsumed = 0;
        switch (commandChar)
        {
        default:
			//comando non riconosciuto, cancello # che e' in posizione 0 cosi'
			//al prossimo giro viene buttato via tutto fino al prossimo #
			nBytesConsumed = 1;
			logger->log ("esapi::Protocol() => invalid command char [%c]\n", commandChar);
            break;

        case 'A':   nBytesConsumed = priv_rs232_handleCommand_A (rs232BufferIN);    break;
        case 'C':   nBytesConsumed = priv_rs232_handleCommand_C (rs232BufferIN);    break;
		case 'S':   nBytesConsumed = priv_rs232_handleCommand_S (rs232BufferIN);    break;

        case 'R':
            //questo tipo di comandi (classe 'R') devono essere gestiti in autonomia dai singoli moduli
            return false;
        }

		rs232BufferIN.removeFirstNBytes(nBytesConsumed);
    }

    return true;
}

/*********************************************************
 * ritorna il numero di bytes consumati
 */
u32 Protocol::priv_rs232_handleCommand_A (const sBuffer &b)
{
    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == 'A');
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol() => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		return 2;

    case '1': 
        //Request API version
        //ricevuto: # A 1 [ck]
        //rispondo: # A 1 [api_ver_major] [api_ver_minor] [GPUmodel] [ck]
		{
			//parse del messaggio
			if (b.buffer[3] != rhea::utils::simpleChecksum8_calc (b.buffer, 3))
				return 2;

			//rispondo.
			const u8 data[4] = { ESAPI_VERSION_MAJOR, ESAPI_VERSION_MINOR, (u8)esapi::eGPUType::TS, 0 };
			rs232_esapiSendAnswer ('A', '1', data, 3);
		}
		return 4;

	case '2':
		//Request Machine ID
		//ricevuto: # A 2 [ck]
        //rispondo: # A 2 'R' 'V' 'H' [id101 come stringa di 9 ASCCI char] [ck]
		{
			//parse del messaggio
			if (b.buffer[3] != rhea::utils::simpleChecksum8_calc (b.buffer, 3))
				return 2;

            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_ID101 (*cpuBridgeSubscriber, 0x01);
		}
		return 4;
	}
}

/********************************************************
 * ritorna il numero di bytes consumati
 */
u32 Protocol::priv_rs232_handleCommand_C (const sBuffer &b)
{
    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == 'C');
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		return 2;

    case '1': 
        //Query CPU screen message
        //ricevuto: # C 1 [ck]
        //rispondo: # C 1 [numByteInMsg] [messageUTF16_LSB_MSB] [ck]
        {
			//parse del messaggio
			if (b.buffer[3] != rhea::utils::simpleChecksum8_calc (b.buffer, 3))
				return 2;
			       
            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_LCD_MESSAGE (*cpuBridgeSubscriber, 0x01);
        }
		return 4;
		
    case '2': 
        //Query selections availability
        //ricevuto: # C 2 [ck]
        //rispondo: # C 2 [avail1-8] [avail9-16] [avail17-24] [...] [avail121-128] [ck]
        {
			//parse del messaggio
			if (b.buffer[3] != rhea::utils::simpleChecksum8_calc (b.buffer, 3))
				return 2;

            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_SEL_AVAIL (*cpuBridgeSubscriber, 0x01);
        }
		return 4;

	case '3':
        //Query selections prices
        //ricevuto: # C 3 [selNum] [ck]
        //rispondo: # C 3 [selNum] [priceLen] [price?] [ck]
        {
            if (b.numBytesInBuffer < 5)	//devo avere almeno 5 char nel buffer
                return 0;

            if (b.buffer[4] != rhea::utils::simpleChecksum8_calc (b.buffer, 4))
                return 2;

            const u8 selNum = b.buffer[3];
			//chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_SINGLE_SEL_PRICE (*cpuBridgeSubscriber, 0x01, selNum);
		}
		return 4;

    case '4':
        //Query 12 LED status
        //ricevuto: # C 4 [ck]
        //rispondo: # C 4 [b1] [b2] [ck]
		{
			//questo comando non è supportato per le TS
			const u8 data[2] = { 0, 0 };
			rs232_esapiSendAnswer ('C', '4', data, 2);
		}
		return 4;

	case '5':
		//Get selection name
		//ricevuto: # C 5 [selNum] [ck]
		//rispondo:	# C 5 [selNum] [nameLenInBytes] [nameUTF16_LSB_MSB...] [ck]
		{
			if (b.numBytesInBuffer < 5)	//devo avere almeno 5 char nel buffer
				return 0;

			if (b.buffer[4] != rhea::utils::simpleChecksum8_calc (b.buffer, 4))
				return 2;

			u8 selNumber = b.buffer[3];
			if (selNumber < 1 || selNumber > 48)
				selNumber = 1;

			//chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
			cpubridge::ask_CPU_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB (*cpuBridgeSubscriber, 0x01, selNumber);
		}
		return 5;

	case '6':
		//Get Cup Sensor Status
		//ricevuto: # C 6 [ck]
		//rispondo: # C 6 [status] [ck] 
		//				dove [status] ='Y' oppure 'N'
		{
			//chiedo a CPUBridge il suo stato perchè l'informazione sullo stato del cup-sensor ce l'ha lei.
			//Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi onMsgFromCPUBridge)
			cpubridge::ask_CPU_QUERY_FULLSTATE (*cpuBridgeSubscriber, 0x0001);
		}
		return 4;
		

    } //switch
}

/********************************************************
 * ritorna il numero di bytes consumati
 */
u32 Protocol::priv_rs232_handleCommand_S (const sBuffer &b)
{
    assert (b.numBytesInBuffer >= 3 && b.buffer[0] == '#' && b.buffer[1] == 'S');
    const u8 commandCode = b.buffer[2];

    switch (commandCode)
    {
    default:
        logger->log ("esapi::Protocol => invalid commandNum [%c][%c]\n", b.buffer[1], commandCode);
		return 2;

    case '1': 
        //Start selection
        //ricevuto: # S 1 [sel_num] [ck]
        //rispondo: # S 1 [sel_num] [ck]
        {
			if (b.numBytesInBuffer < 5)	//devo avere almeno 5 char nel buffer
				return 0;

			if (b.buffer[4] != rhea::utils::simpleChecksum8_calc (b.buffer, 4))
				return 2;

			u8 selNumber = b.buffer[3];
			if (selNumber < 1 || selNumber > 48)
				selNumber = 0;

            //chiedo a CPUBridge di iniziare la selezione indicata. CPUBridge risponderà con una serie di notify_CPU_RUNNING_SEL_STATUS() per
            //indicare lo stato di avanzamento della selezione
			if (selNumber > 0)
			{
				runningSel.status = cpubridge::eRunningSelStatus::wait;
				cpubridge::ask_CPU_START_SELECTION (*cpuBridgeSubscriber, selNumber);
			}

			//rispondo
			rs232_esapiSendAnswer ('S', '1', &selNumber, 1);
        }
		return 5;

    case '2': 
        //Query selection status
        //ricevuto: # S 2 [ck]
        //rispondo: # S 2 [status] [ck]
		//		[status] can be one of the following:
		//		0x01 => waiting for payment
		//		0x02 => delivering in progress, STOP button is not available
		//		0x03 => finished KO
		//		0x04 => finished OK
		//		0x05 => delivering in progress, STOP button is available		
		{
			if (b.buffer[3] != rhea::utils::simpleChecksum8_calc (b.buffer, 3))
				return 2;

            //rispondo
			const u8 status = (u8)runningSel.status;
			rs232_esapiSendAnswer ('S', '2', &status, 1);
        }
		return 4;

	case '3':
		//start already paid selection
		//ricevo:	# S 3 [sel_num] [priceLSB] [priceMSB] [ck]
		//rispondo:	# S 3 [sel_num] [ck]
		{
			if (b.numBytesInBuffer < 7)	//devo avere almeno 7 char nel buffer
				return 0;

			if (b.buffer[6] != rhea::utils::simpleChecksum8_calc (b.buffer, 6))
				return 2;

			//chiedo a CPUBridge di iniziare la selezione indicata. CPUBrdige risponderà con una serie di notify_CPU_RUNNING_SEL_STATUS() per
			//indicare lo stato di avanzamento della selezione
			u8 selNum = b.buffer[3];
			if (selNum < 1 || selNum > 48)
				selNum = 0;
			else
			{
				const u16 price = rhea::utils::bufferReadU16_LSB_MSB(&b.buffer[4]);
				runningSel.status = cpubridge::eRunningSelStatus::wait;
				cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (*cpuBridgeSubscriber, selNum, price, cpubridge::eGPUPaymentType::unknown);
			}

			//rispondo
			rs232_esapiSendAnswer ('S', '3', &selNum, 1);
		}
		return 7;

	case '4':
		//Send button press
        //ricevuto: # S 4 [btn] [ck]
        //rispondo: # S 4 [btn] [ck]
		{
			if (b.numBytesInBuffer < 5)	//devo avere almeno 5 char nel buffer
				return 0;

			if (b.buffer[4] != rhea::utils::simpleChecksum8_calc (b.buffer, 4))
				return 2;

			u8 btnNum =b.buffer[3];

			//inoltro a CPUBridge
			if (btnNum > 0 && btnNum <= 12)
				cpubridge::ask_CPU_SEND_BUTTON (*cpuBridgeSubscriber, btnNum);
			else
				btnNum = 0;

			//rispondo
			rs232_esapiSendAnswer ('S', '4', &btnNum, 1);
		}
		return 5;
	} //switch
}

