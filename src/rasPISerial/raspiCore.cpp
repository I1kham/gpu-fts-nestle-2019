#include "raspiCore.h"
#include "../rheaExternalSerialAPI/ESAPI.h"
#include "../rheaCommonLib/rheaAllocatorSimple.h"
#include "../rheaCommonLib/rheaUtils.h"

using namespace raspi;

//*******************************************************
Core::Core ()
{
	localAllocator = NULL;
	logger = &nullLogger;
	sok2280NextID = 0x00;
	rs232BufferOUT = NULL;
	rhea::rs232::setInvalid (com);
	rhea::socket::init (&sok2280);
}

//***************************************************
void Core::useLogger (rhea::ISimpleLogger *loggerIN)
{
	if (NULL == loggerIN)
		logger = &nullLogger;
	else
		logger = loggerIN;
}

//*******************************************************
bool Core::open (const char *serialPort)
{
	logger->log ("opening com=%s   ", serialPort);
	if (!rhea::rs232::open(&com, serialPort, eRS232BaudRate_115200, false, false, eRS232DataBits_8, eRS232Parity_No, eRS232StopBits_One, eRS232FlowControl_No, false))
	{
		logger->log ("FAILED. unable to open port [%s]\n", serialPort);
		logger->decIndent();
		return false;
	}
	logger->log ("OK\n");
	

	logger->log ("opening socket on 2280...");
	eSocketError err = rhea::socket::openAsTCPServer(&sok2280, 2280);
	if (err != eSocketError_none)
	{
		logger->log ("ERR code[%d]\n", err);
		logger->log("\n");
		return false;
	}
	logger->log("OK\n");

	rhea::socket::setReadTimeoutMSec(sok2280, 0);
	rhea::socket::setWriteTimeoutMSec(sok2280, 10000);

	logger->log("listen... ");
	if (!rhea::socket::listen(sok2280))
	{
		logger->log("FAIL\n", err);
		logger->decIndent();
		rhea::socket::close(sok2280);
		return false;
	}
	logger->log("OK\n");


	//aggiungo la socket al gruppo di oggetti in osservazione
	waitableGrp.addSocket(sok2280, WAITGRP_SOCKET2280);

	//buffer vari
	localAllocator = RHEANEW(rhea::getSysHeapAllocator(), rhea::AllocatorSimpleWithMemTrack) ("raspiCore");
	rs232BufferOUT = (u8*)RHEAALLOC(localAllocator, RS232_BUFFEROUT_SIZE);
	rs232BufferIN.alloc (localAllocator, 4096);
	sok2280Buffer = (u8*)RHEAALLOC(localAllocator, SOK_BUFFER_SIZE);
	clientList.setup (localAllocator, 128);

	return true;
}

//*****************************************************************
void Core::priv_close ()
{
	rhea::rs232::close (com);
	rhea::socket::close (sok2280);

	if (localAllocator)
	{
		RHEAFREE(localAllocator, rs232BufferOUT);
		rs232BufferIN.free (localAllocator);
		RHEAFREE(localAllocator, sok2280Buffer);
		clientList.unsetup();
		RHEADELETE(rhea::getSysHeapAllocator(), localAllocator);
		localAllocator = NULL;
	}
}

/*******************************************************
 *	In questa fase, invio periodicamente alla GPU il comando #A1 per conoscere la versione di API supportata e per capire quando la GPU
 *	diventa disponibile.
 *	Fino a che non ricevo risposta, rimango in questo loop
 */
void Core::priv_identify()
{
	reportedESAPIVerMajor = 0;
	reportedESAPIVerMinor = 0;
	reportedGPUType = esapi::eGPUType_unknown;

	u64 timeToSendMsgMSec = 0;
	while (1)
	{
		const u64 timeNowMSec = rhea::getTimeNowMSec();
		if (timeNowMSec >= timeToSendMsgMSec)
		{
			timeToSendMsgMSec = timeNowMSec + 2000;
			logger->log ("requesting API version...\n");

			const u32 nBytesToSend = esapi::buildMsg_A1_getAPIVersion_ask (rs232BufferOUT, RS232_BUFFEROUT_SIZE);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			priv_rs232_handleIncomingData (rs232BufferIN);

			if (reportedESAPIVerMajor != 0)
			{
				//ok, la GPU mi ha risposto, quindi è viva.
				//Segnalo la mia identità
				const u32 n = esapi::buildMsg_R1_externalModuleIdentify_ask (esapi::eExternalModuleType_rasPI_wifi_REST, VER_MAJOR, VER_MINOR, rs232BufferOUT, RS232_BUFFEROUT_SIZE);
				priv_rs232_sendBuffer (rs232BufferOUT, n);
				priv_rs232_handleIncomingData (rs232BufferIN);
				return;
			}
		}

		rhea::thread::sleepMSec(100);
	}
}

//*******************************************************
void Core::run()
{
	priv_identify();


	while (1)
	{
		const u8 nEvents = waitableGrp.wait(100);
		for (u8 i = 0; i < nEvents; i++)
		{
			if (waitableGrp.getEventOrigin(i) == OSWaitableGrp::evt_origin_socket)
			{
				if (waitableGrp.getEventUserParamAsU32(i) == WAITGRP_SOCKET2280)
				{
					//evento generato dalla socket in listen sulla 2280
					priv_2280_accept();
				}
				else
				{
					//altimenti la socket che si è svegliata deve essere una dei miei client già connessi
					const u32 clientUID = waitableGrp.getEventUserParamAsU32(i);
					OSSocket sok = waitableGrp.getEventSrcAsOSSocket (i);
					priv_2280_onIncomingData (sok, clientUID);
				}
			}
		}

		priv_rs232_handleIncomingData(rs232BufferIN);
	}
}

//*********************************************************
void Core::priv_rs232_sendBuffer (const u8 *buffer, u32 numBytesToSend)
{
	rhea::rs232::writeBuffer (com, buffer, numBytesToSend);
}

//*********************************************************
void Core::priv_rs232_handleIncomingData (sBuffer &b)
{
	while (1)
	{
		//leggo tutto quello che posso dalla seriale e bufferizzo in [b]
		const u16 nBytesAvailInBuffer = (u16)(b.SIZE - b.numBytesInBuffer);
		if (nBytesAvailInBuffer > 0)
		{
			const u32 nRead = rhea::rs232::readBuffer(com, &b.buffer[b.numBytesInBuffer], nBytesAvailInBuffer);
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

		case 'A':   if (!priv_rs232_handleCommand_A (b)) return;    break;
		case 'R':   if (!priv_rs232_handleCommand_R (b)) return;    break;
		}

	} //while(1)

}

/*********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_rs232_handleCommand_A (sBuffer &b)
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
        //risposta di GPU alla mia richiesta di API Version (A1)
		{
			bool bValidCk = false;
			if (!esapi::buildMsg_A1_getAPIVersion_parseResp (b.buffer, b.numBytesInBuffer, &bValidCk, &this->reportedESAPIVerMajor, &this->reportedESAPIVerMinor, &reportedGPUType))
				return false;

			if (!bValidCk)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			logger->log ("reported ESAPI version [%d].[%d], GPUType[%d]\n", reportedESAPIVerMajor, reportedESAPIVerMinor, reportedGPUType);

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(7);
		}
		return true;
        break;
    }
}

/********************************************************
 * ritorna true se ha consumato qualche byte di buffer.
 * ritorna false altrimenti. In questo caso, vuol dire che i byte in buffer sembravano essere un valido messaggio ma probabilmente manca ancora qualche
 *  bytes al completamento del messaggio stesso. Bisogna quindi attendere che ulteriori byte vengano appesi al buffer
 */
bool Core::priv_rs232_handleCommand_R (Core::sBuffer &b)
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

	case 0x02:
		//GPU mi comunica che la socket xxx è stata chiusa
		{
           //parse del messaggio
            bool bValidCk = false;
			u32 socketUID;
            const u32 MSG_LEN = esapi::buildMsg_R0x02_closeSocket_parse (b.buffer, b.numBytesInBuffer, &bValidCk, &socketUID);
            if (0 == MSG_LEN)
                return false;
            if (!bValidCk)
            {
                b.removeFirstNBytes(2);
                return true;
            }
        
            //rimuovo msg dal buffer
            b.removeFirstNBytes(MSG_LEN);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findClientByUID (socketUID);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, socketUID);
		}
		return true;

	case 0x04:
		//GPU mi sta comunicando dei dati che io devo mandare lungo la socket indicata
		//rcv:   # R [0x04] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [lenMSB] [lenLSB] [data…] [ck]
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
				sConnectedSocket *cl = priv_2280_findClientByUID(uid);
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
			b.removeFirstNBytes(10+dataLen);
			return true;
		}
		break;
	}
}


//*******************************************************
Core::sConnectedSocket* Core::priv_2280_findClientByUID (u32 uid)
{
	for (u32 i = 0; i < clientList.getNElem(); i++)
	{
		if (clientList(i).uid == uid)
			return &clientList[i];
	}
	return NULL;
}

//*******************************************************
void Core::priv_2280_accept()
{
	sConnectedSocket cl;
	if (!rhea::socket::accept (sok2280, &cl.sok))
	{
		logger->log("ERR => accept failed on 2280\n");
		return;
	}

	//ok, ho accettato una socket
	//Gli assegno un id univoco
	cl.uid = ++sok2280NextID;

	//comunico via seriale che ho accettato una nuova socket
	const u32 nBytesToSend = esapi::buildMsg_R0x01_newSocket (cl.uid, rs232BufferOUT, RS232_BUFFEROUT_SIZE);
	priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);

	//aggiungo la socket alla lista dei client connessi
	clientList.append (cl);
	waitableGrp.addSocket (cl.sok, cl.uid);

	logger->log ("socket [%d] connected\n", cl.uid);
}

/*********************************************************
 * Ho ricevuto dei dati lungo la socket, li spedisco via rs232 alla GPU
 */
void Core::priv_2280_onIncomingData (OSSocket &sok, u32 uid)
{
	sConnectedSocket *cl = priv_2280_findClientByUID(uid);
	if (NULL == cl)
	{
		DBGBREAK;
		return;
	}
	assert (rhea::socket::compare(cl->sok, sok));


	i32 nBytesLetti = rhea::socket::read (cl->sok, sok2280Buffer, SOK_BUFFER_SIZE, 100);
	if (nBytesLetti == 0)
	{
		//connessione chiusa
		priv_2280_onClientDisconnected(sok, uid);
		return;
	}
	if (nBytesLetti < 0)
	{
		//la chiamata sarebbe stata bloccante, non dovrebbe succedere
		DBGBREAK;
		return;
	}

	//spedisco lungo la seriale
	const u32 nBytesToSend = esapi::buildMsg_R0x03_socketDataToGPU (cl->uid, sok2280Buffer, nBytesLetti, rs232BufferOUT, RS232_BUFFEROUT_SIZE);
	priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
	logger->log ("rcv [%d] bytes from socket [%d], sending to GPU\n", nBytesLetti, cl->uid);

}

//*********************************************************
void Core::priv_2280_onClientDisconnected (OSSocket &sok, u32 uid)
{
	for (u32 i = 0; i < clientList.getNElem(); i++)
	{
		if (clientList(i).uid == uid)
		{
			assert (rhea::socket::compare(clientList(i).sok, sok));

			waitableGrp.removeSocket (clientList[i].sok);
			rhea::socket::close(clientList[i].sok);
			clientList.removeAndSwapWithLast(i);

			//comunico la disconnessione via seriale
			const u32 nBytesToSend = esapi::buildMsg_R0x02_closeSocket (uid, rs232BufferOUT, RS232_BUFFEROUT_SIZE);
			priv_rs232_sendBuffer (rs232BufferOUT, nBytesToSend);
			logger->log ("socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;

}