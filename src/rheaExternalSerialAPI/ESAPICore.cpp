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
    answerBuffer = NULL;
}

//*********************************************************
void Core::priv_allocBuffer(Core::sBuffer *out, u16 max_size)
{
	out->SIZE = max_size;
	out->numBytesInBuffer = 0;
	out->buffer = (u8*)RHEAALLOC(localAllocator, max_size);
}

//*********************************************************
void Core::priv_freeBuffer (Core::sBuffer &b)
{
	b.numBytesInBuffer = 0;
	b.SIZE = 0;
	if (NULL != b.buffer)
		RHEAFREE(localAllocator, b.buffer);
	b.buffer = NULL;
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
    priv_allocBuffer (&serialBuffer, 2048);
    answerBuffer = (u8*)RHEAALLOC(localAllocator, SIZE_OF_ANSWER_BUFFER);

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

        priv_freeBuffer (serialBuffer);
        RHEAFREE(localAllocator, answerBuffer);
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
        const u8 nEvents = waitableGrp.wait(100);
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

			default:
                logger->log("esapi::Core::run() => unhandled event from waitGrp [%d]\n", waitableGrp.getEventOrigin(i));
				break;
			}
		}

        //gestione comunicazione seriale
        priv_handleSerialCommunication(com, serialBuffer);
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
                //to send:   # C 1 [len_LSB_MSB] [messageUTF16_LSB_MSB] [ck]
                {
	                cpubridge::sCPULCDMessage lcdMsg;
	                translateNotify_CPU_NEW_LCD_MESSAGE(msg, &lcdMsg);

	                const u16 msgLenInBytes = rhea::string::utf16::lengthInBytes(lcdMsg.utf16LCDString);

                    u16 ct = 0;
                    answerBuffer[ct++] = '#';
                    answerBuffer[ct++] = 'C';
                    answerBuffer[ct++] = '1';
                    rhea::utils::bufferWriteU16_LSB_MSB (&answerBuffer[ct], msgLenInBytes);
                    ct += 2;
                    if (msgLenInBytes > 0)
                    {
                        memcpy(&answerBuffer[ct], lcdMsg.utf16LCDString, msgLenInBytes);
                        ct += msgLenInBytes;
                    }

                    answerBuffer[ct] = esapi::calcChecksum (answerBuffer, ct-1);
                    priv_sendBuffer (com, answerBuffer, ct+1);
                }
                break;


            case CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED:
                //risposta al comando # C 2
                //to send:   # C 2 [avail1-8] [avail9-16] [avail17-24] [...] [avail121-128] [ck]
                {
	                cpubridge::sCPUSelAvailability selAvail;
	                cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(msg, &selAvail);

                    answerBuffer[0] = '#';
                    answerBuffer[1] = 'C';
                    answerBuffer[2] = '2';
                    memset (&answerBuffer[3], 0x00, 16);
	                for (u8 i = 1; i <= NUM_MAX_SELECTIONS; i++)
	                {
                        if (selAvail.isAvail(i))
                            rhea::bit::set (&answerBuffer[3], 16, i-1);
	                }
                    answerBuffer[19] = esapi::calcChecksum (answerBuffer, 19);
                    priv_sendBuffer (com, answerBuffer, 20);
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
void Core::priv_handleSerialCommunication (OSSerialPort &comPort, Core::sBuffer &b)
{
    while (1)
    {
	    //leggo tutto quello che posso dalla seriale e bufferizzo in [b]
        const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);
	    if (nBytesAvailInBuffer > 0)
	    {
		    const u32 nRead = rhea::rs232::readBuffer(comPort, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
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
            logger->log ("invalid command char [%c]\n", commandChar);
            b.removeFirstNBytes(1);
            break;

        case 'A':   if (!priv_handleCommand_A (comPort, b)) return;    break;
        case 'C':   if (!priv_handleCommand_C (comPort, b)) return;    break;
        case 'S':   if (!priv_handleCommand_S (comPort, b)) return;    break;
            break;

        }

    } //while(1)

}

//*********************************************************
void Core::priv_sendBuffer (OSSerialPort &comPort, const u8 *buffer, u32 numBytesToSend)
{
    rhea::rs232::writeBuffer (comPort, buffer, numBytesToSend);
}

//*********************************************************
void Core::priv_buildAndSendAnswer (OSSerialPort &comPort, u8 commandChar, const u8* optionalData, u32 numOfBytesInOptionalData)
{
    u8 ct = 0;
    answerBuffer[ct++] = '#';
    answerBuffer[ct++] = commandChar;
    if (NULL != optionalData && numOfBytesInOptionalData)
    {
        memcpy (&answerBuffer[ct], optionalData, numOfBytesInOptionalData);
        ct += numOfBytesInOptionalData;
    }

    answerBuffer[ct] = esapi::calcChecksum (answerBuffer, ct);
    ct++;

    priv_sendBuffer (comPort, answerBuffer, ct);
}

/*********************************************************
    ritorna true se nel buffer c'è un valido messaggio. Un valido messaggio è lungo [expectedCommandLen] bytes e la sua checksum è valida.
    ritorna false altrimenti.

    [out_atLeastOneByteConsumed] viene messo a true se la fn consuma almeno un byte di buffer
*/
bool Core::priv_utils_parseCommand (sBuffer &b, u32 expectedCommandLen, bool *out_atLeastOneByteConsumed)
{
    assert (b.numBytesInBuffer >= 2 & b.buffer[0] == '#');

    *out_atLeastOneByteConsumed = false;
    
    //ci devono essere almeno [expectedCommandLen] byte nel buffer, altrimenti devo aspettare che ne arrivino altri
    if (b.numBytesInBuffer < expectedCommandLen)
        return false;

    //ok, posto che ci sono i byte, vediamo se la ck è valida
    if (!esapi::isValidChecksum(b.buffer[expectedCommandLen-1], b.buffer, expectedCommandLen-1))
    {
        *out_atLeastOneByteConsumed = true;
        b.removeFirstNBytes(2);
        return false;
    }

    //abbiamo un messaggio completo e con la ck corretta
    return true;
}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_handleCommand_A (OSSerialPort &comPort, sBuffer &b)
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
        //API version
        //rcv:      # A 1 [ck]  ck=149
        //answer:   # A 1 [api_ver_major] [api_ver_minor] [ck]
        {
            const u8 MSGLEN = 4;
            if (!priv_utils_parseCommand(b, MSGLEN, &ret))
                return ret;

            //rispondo
            const u8 data[3] = { commandCode, API_VERSION_MAJOR, API_VERSION_MINOR };
            priv_buildAndSendAnswer (comPort, COMMAND_CHAR, data, sizeof(data));
            b.removeFirstNBytes(MSGLEN);
            return true;
        }
        break;
    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Core::priv_handleCommand_C (OSSerialPort &comPort, sBuffer &b)
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
        //Query CPU screen message
        //rcv:      # C 1 [ck]   ck=151
        //answer:   # C 1 [len_LSB_MSB] [messageUTF16_LSB_MSB] [ck]
        {
            const u8 MSGLEN = 4;
            if (!priv_utils_parseCommand(b, MSGLEN, &ret))
                return ret;

            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi priv_handleIncomingMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_LCD_MESSAGE (this->cpuBridgeSubscriber, 0x01);

            b.removeFirstNBytes(MSGLEN);
            return true;
        }
        break;

    case '2': 
        //Get selection availability
        //rcv:      # C 2 [ck] ck=152
        //answer:   # C 2 [avail1-8] [avail9-16] [avail17-24] [...] [avail121-128] [ck]
        {
            const u8 MSGLEN = 4;
            if (!priv_utils_parseCommand(b, MSGLEN, &ret))
                return ret;

            //chiedo a CPUBridge. Alla ricezione della risposta da parte di CPUBridge, rispondo a mia volta lungo la seriale (vedi priv_handleIncomingMsgFromCPUBridge)
            cpubridge::ask_CPU_QUERY_SEL_AVAIL (this->cpuBridgeSubscriber, 0x01);

            b.removeFirstNBytes(MSGLEN);
            return true;
        }
        break;

    }
}

/********************************************************
 * funziona come priv_mode_raw_handleCommand_A(), vedi i commenti in cima a quella fn
 */
bool Core::priv_handleCommand_S (OSSerialPort &comPort, sBuffer &b)
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
        //Start selection
        //rcv:      # S 1 [sel_num] [ck]
        //answer:   # S 1 [sel_num] [ck]
        {
            const u8 MSGLEN = 5;
            if (!priv_utils_parseCommand(b, MSGLEN, &ret))
                return ret;

            //chiedo a CPUBridge di iniziare la selezione indicata. CPUBrdige risponderà con una serie di notify_CPU_RUNNING_SEL_STATUS() per
            //indicare lo stato di avanzamento della selezione
            runningSel.status = cpubridge::eRunningSelStatus_wait;
            const u8 selNumber = b.buffer[2];
            const u16 price = 0xffff; //serve per fare in modo che la CPU gestisca lei il pagamento
            cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (this->cpuBridgeSubscriber, selNumber, price, cpubridge::eGPUPaymentType_unknown);

            //rispondo via seriale confermando di aver ricevuto il msg
            priv_sendBuffer (comPort, b.buffer, MSGLEN);
            b.removeFirstNBytes(MSGLEN);
            return true;
        }
        break;

    case '2': 
        //Query selection status
        //rcv:      # S 2 [ck]
        //answer:   # S 2 [status] [ck]
        {
            const u8 MSGLEN = 4;
            if (!priv_utils_parseCommand(b, MSGLEN, &ret))
                return ret;

            //rispondo
            const u8 data[2] = { commandCode, (u8)runningSel.status };
            priv_buildAndSendAnswer (comPort, COMMAND_CHAR, data, 2);
            b.removeFirstNBytes(MSGLEN);
            return true;
        }
        break;

    }
}