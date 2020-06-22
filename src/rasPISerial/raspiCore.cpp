#include "raspiCore.h"
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

//*******************************************************
void Core::run()
{
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
void Core::priv_rs232_buildAndSendMsg (eRSR232Op command, const u8* optionalData, u32 numOfBytesInOptionalData)
{
	u8 ct = 0;
	rs232BufferOUT[ct++] = '#';
	rs232BufferOUT[ct++] = 'R';
	rs232BufferOUT[ct++] = (u8)command;
	if (NULL != optionalData && numOfBytesInOptionalData)
	{
		memcpy (&rs232BufferOUT[ct], optionalData, numOfBytesInOptionalData);
		ct += numOfBytesInOptionalData;
	}

	rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
	ct++;

	priv_rs232_sendBuffer (rs232BufferOUT, ct);
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

		case 'R':   if (!priv_rs232_handleCommand_R (b)) return;    break;
		}

	} //while(1)

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
		//rcv:	# R [0x02] [client_uid_MSB3] [client_uid_MSB2] [client_uid_MSB1] [client_uid_LSB] [ck]
		{
			if (b.numBytesInBuffer < 8)
				return false;

			const u32 uid = rhea::utils::bufferReadU32 (&b.buffer[3]);
			const u8 ck = b.buffer[7];
			if (rhea::utils::simpleChecksum8_calc(b.buffer, 7) != ck)
			{
				b.removeFirstNBytes(2);
				return true;
			}

			//rimuovo il msg dal buffer di input
			b.removeFirstNBytes(8);

			//elimino il client
			sConnectedSocket *cl = priv_2280_findClientByUID (uid);
			if (cl)
				priv_2280_onClientDisconnected (cl->sok, uid);
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
	u8 data[8];
	rhea::utils::bufferWriteU32 (data, cl.uid);
	priv_rs232_buildAndSendMsg (eRSR232Op_socketAccept, data, 4);

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
	u8 ct = 0;
	rs232BufferOUT[ct++] = '#';
	rs232BufferOUT[ct++] = 'R';
	rs232BufferOUT[ct++] = (u8)eRSR232Op_socketRcv;

	rhea::utils::bufferWriteU32 (&rs232BufferOUT[ct], cl->uid);
	ct += 4;

	rhea::utils::bufferWriteU16 (&rs232BufferOUT[ct], nBytesLetti);
	ct += 2;

	memcpy (&rs232BufferOUT[ct], sok2280Buffer, nBytesLetti);
	ct += nBytesLetti;

	rs232BufferOUT[ct] = rhea::utils::simpleChecksum8_calc (rs232BufferOUT, ct);
	ct++;

	priv_rs232_sendBuffer (rs232BufferOUT, ct);
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
			u8 data[8];
			rhea::utils::bufferWriteU32(data, uid);
			priv_rs232_buildAndSendMsg (eRSR232Op_socketClose, data, 4);

			logger->log ("socket [%d] disconnected\n", uid);
			return;
		}
	}

	//non ho trovato il client nella lista...
	DBGBREAK;

}