#include "stress.h"


//**********************************************
Stress::Stress()
{}

//**********************************************
Stress::~Stress()
{}

//**********************************************
void Stress::setup(HThreadMsgR &msgQHandleR, rhea::ProtocolConsole *proto, rhea::ProtocolChSocketTCP *ch, rhea::LinearBuffer *bufferR, OSWaitableGrp *waitGrp, rhea::ISimpleLogger *logger)
{
	this->msgQHandleR = msgQHandleR;
	this->proto = proto;
	this->ch = ch;
	this->bufferR = bufferR;
	this->waitGrp = waitGrp	;
	this->logger = logger;
}


//*****************************************************
bool Stress::priv_onMsgRcv (rhea::app::sDecodedMsg &decoded)
{
	while (1)
	{
		u16 nRead = proto->read(ch, 100, *bufferR);
		if (nRead == 0)
			return false;
		if (nRead >= rhea::protocol::RES_ERROR)
			return false;

		u8 *buffer = bufferR->_getPointer(0);
		u16 nBytesUsed = 0;
		while (rhea::app::decodeSokBridgeMessage(buffer, nRead, &decoded, &nBytesUsed))
		{
			switch (decoded.what)
			{
			case rhea::app::eDecodedMsgType_event:
				return true;
				break;

			default:
				DBGBREAK;
				break;
			}

			assert(nRead >= nBytesUsed);
			nRead -= nBytesUsed;
			if (nRead > 0)
				buffer += nBytesUsed;
		}
	}
}

//**********************************************
u16 Stress::downloadDA3()
{
	//chiedo di leggere il DA3
	logger->log("downloading");
	rhea::app::ReadVMCDataFile::ask(ch, proto);

	//processo i messaggi in arrivo
	while (1)
	{
		//ogni 10 secondi mi sblocco indipendentemente dall'avere ricevuto notifiche o meno
		u8 nEvent = waitGrp->wait(10000);

		//vediamo cosa mi ha svegliato
		for (u8 i = 0; i < nEvent; i++)
		{
			if (OSWaitableGrp::evt_origin_socket == waitGrp->getEventOrigin(i))
			{
				//ho qualcosa sulla socket
				rhea::app::sDecodedMsg decoded;
				if (priv_onMsgRcv(decoded))
				{
					rhea::app::sDecodedEventMsg *ev = &decoded.data.asEvent;
					if (ev->eventType == socketbridge::eEventType_reqVMCDataFile)
					{
						cpubridge::eReadDataFileStatus status;
						u16 toKbSoFar;
						u16 fileID;
						rhea::app::ReadVMCDataFile::decodeAnswer(*ev, &status, &toKbSoFar, &fileID);
						//logger->log("readVMCDataFile: status[%s] totKbSoFar[%d] fileID[%d]\n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);

						if (status == cpubridge::eReadDataFileStatus_finishedOK)
						{
							logger->log("\n");
							return fileID;
						}
						else if (status == cpubridge::eReadDataFileStatus_inProgress)
						{
							logger->log(".");
						}
						else
						{
							//Errore
							return u16MAX;
						}
					}
				}
			}
		}
	}

}

//**********************************************
bool Stress::uploadDA3 (const char *filename)
{
	//chiedo di leggere il DA3
	logger->log("uploading");
	rhea::app::WriteLocalVMCDataFile::ask (ch, proto, filename);

	//processo i messaggi in arrivo
	while (1)
	{
		//ogni 10 secondi mi sblocco indipendentemente dall'avere ricevuto notifiche o meno
		u8 nEvent = waitGrp->wait(10000);

		//vediamo cosa mi ha svegliato
		for (u8 i = 0; i < nEvent; i++)
		{
			if (OSWaitableGrp::evt_origin_socket == waitGrp->getEventOrigin(i))
			{
				//ho qualcosa sulla socket
				rhea::app::sDecodedMsg decoded;
				if (priv_onMsgRcv(decoded))
				{
					rhea::app::sDecodedEventMsg *ev = &decoded.data.asEvent;
					if (ev->eventType == socketbridge::eEventType_reqWriteLocalVMCDataFile)
					{
						cpubridge::eWriteDataFileStatus status;
						u16 toKbSoFar;
						rhea::app::WriteLocalVMCDataFile::decodeAnswer(*ev, &status, &toKbSoFar);
						//logger->log("readVMCDataFile: status[%s] totKbSoFar[%d] fileID[%d]/n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);

						if (status == cpubridge::eWriteDataFileStatus_finishedOK)
						{
							logger->log("\n");
							return true;
						}
						else if (status == cpubridge::eWriteDataFileStatus_inProgress)
						{
							logger->log(".");
						}
						else
						{
							//Errore
							return false;
						}
					}
				}
			}
		}
	}
}
