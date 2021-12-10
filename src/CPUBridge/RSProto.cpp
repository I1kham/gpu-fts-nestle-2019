#include "RSProto.h"
#include "CPUBridgeServer.h"
#include "rhFSProtocol.h"
#include "../rheaCommonLib/rheaUtils.h"

//**********************************************************
RSProto::RSProto (rhea::ProtocolSocketServer *serverTCPIN, const HSokServerClient hClientIN)
{
	serverTCP = serverTCPIN;
	hClient = hClientIN;
	memset (lastStatus, 0, sizeof(lastStatus));
	priv_reset();
}

//**********************************************************
RSProto::~RSProto ()
{}

//******************************************* 
void RSProto::priv_reset()
{
	nextTimeSendTemperature_msec = 0;
	memset (lastTemperature, 0xFF, sizeof(lastTemperature));
}

//***************************************************
void RSProto::priv_send (u16 what, u16 userValue, const void *payload, u32 payloadLen)
{
	u8 bufferW[1024];

	const u16 nByteToSend = rhFSx::proto::encodeMsg (what, userValue, payload, payloadLen, bufferW, sizeof(bufferW));
	assert (nByteToSend > 0);

	serverTCP->client_writeBuffer (hClient, bufferW, nByteToSend);
}

//******************************************* 
void RSProto::onSMUStateChanged (const cpubridge::sCPUStatus &status, rhea::ISimpleLogger *logger)
{
	//evito di mandare il msg se non sono informazioni nuove
	const u8 cpuState = static_cast<u8>(status.VMCstate);
	if (lastStatus[0] == cpuState && lastStatus[1] == status.VMCerrorCode && lastStatus[2] == status.VMCerrorType)
		return;
	lastStatus[0] = cpuState;
	lastStatus[1] = status.VMCerrorCode;
	lastStatus[2] = status.VMCerrorType;

	//invio del nuovo stato di VMC
	priv_sendLastSMUState(logger);
}

//******************************************* 
void RSProto::priv_sendLastSMUState (rhea::ISimpleLogger *logger  UNUSED_PARAM)
{
	u8 payload[16];
	u32 ct = 0;

	rhea::utils::bufferWriteU32 (payload, VAR_STATO_MACCHINA);
	ct+=4;
	payload[ct++] = VAR_TYPE_u32;
	payload[ct++] = MACHINE_TYPE_TT;
	payload[ct++] = lastStatus[0];	//cpu state
	payload[ct++] = lastStatus[1];	//cpu error code
	payload[ct++] = lastStatus[2];	//cpu error code type

	//invio al modulo la notifica di cambio stato
	priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, ct);

	//const vmc::eCPUState debug_cpuState = static_cast<vmc::eCPUState>(lastStatus[0]);
	//logger->log (eTextColor::cyan, "RSProto SND SMUState [%d - %s] [err=%d] [grp-err=%d]\n", lastStatus[0], vmcutils::enumToString(debug_cpuState), lastStatus[1], lastStatus[2]);
}

//******************************************* 
void RSProto::sendTemperature (u8 temperatureEsp, u8 temperatureCamCaffe, u8 temperatureSol, u8 temperatureIce, u8 temperatureMilker, rhea::ISimpleLogger *logger UNUSED_PARAM)
{
#define NEED_TO_BE_SENT(newTempValue, oldTempValue, tolleranza)		(newTempValue >= (oldTempValue + tolleranza) || newTempValue <= (oldTempValue - tolleranza))
	
	const cpubridge::eVMCState cpuState = static_cast<cpubridge::eVMCState>(lastStatus[0]);
	if (cpuState == cpubridge::eVMCState::COM_ERROR)
		return;

	u8 payload[16];
	if (NEED_TO_BE_SENT(temperatureEsp, lastTemperature[0], 2))
	{
		lastTemperature[0] = temperatureEsp;
		
		rhea::utils::bufferWriteU32 (payload, VAR_TEMPERATURA_ESPRESSO);
		payload[4] = VAR_TYPE_u32;
		rhea::utils::bufferWriteU32 (&payload[5], lastTemperature[0]);
		//logger->log ("RSProto: send VAR_TEMPERATURA_ESPRESSO [%d]\n", lastTemperature[0]);
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, 9);
	}

	if (NEED_TO_BE_SENT(temperatureCamCaffe, lastTemperature[1], 2))
	{
		lastTemperature[1] = temperatureCamCaffe;
		
		rhea::utils::bufferWriteU32 (payload, VAR_TEMPERATURA_CAM_CAFFE);
		payload[4] = VAR_TYPE_u32;
		rhea::utils::bufferWriteU32 (&payload[5], lastTemperature[1]);
		//logger->log ("RSProto: send VAR_TEMPERATURA_CAM_CAFFE [%d]\n", lastTemperature[1]);
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, 9);
	}


	if (NEED_TO_BE_SENT(temperatureSol, lastTemperature[2], 2))
	{
		lastTemperature[2] = temperatureSol;
		
		rhea::utils::bufferWriteU32 (payload, VAR_TEMPERATURA_SOLUBILE);
		payload[4] = VAR_TYPE_u32;
		rhea::utils::bufferWriteU32 (&payload[5], lastTemperature[2]);
		//logger->log ("RSProto: send VAR_TEMPERATURA_SOLUBILE [%d]\n", lastTemperature[2]);
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, 9);
	}

	if (NEED_TO_BE_SENT(temperatureIce, lastTemperature[3], 2))
	{
		lastTemperature[3] = temperatureIce;
		
		rhea::utils::bufferWriteU32 (payload, VAR_TEMPERATURA_BANCO_GHIACCIO);
		payload[4] = VAR_TYPE_u32;
		rhea::utils::bufferWriteU32 (&payload[5], lastTemperature[3]);
		//logger->log ("RSProto: send VAR_TEMPERATURA_BANCO_GHIACCIO [%d]\n", lastTemperature[3]);
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, 9);
	}

	if (NEED_TO_BE_SENT(temperatureMilker, lastTemperature[4], 2))
	{
		lastTemperature[4] = temperatureMilker;
		
		rhea::utils::bufferWriteU32 (payload, VAR_TEMPERATURA_MILKER);
		payload[4] = VAR_TYPE_u32;
		rhea::utils::bufferWriteU32 (&payload[5], lastTemperature[4]);
		//logger->log ("RSProto: send VAR_TEMPERATURA_MILKER [%d]\n", lastTemperature[4]);
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_variabile), (u16)0, payload, 9);
	}

#undef NEED_TO_BE_SENT
}

//******************************************* 
void RSProto::onMessageRCV (cpubridge::Server *server, u16 what, u16 userValue, const u8 *payload, u32 payloadLen, rhea::ISimpleLogger *logger)
{
	switch (static_cast<rhFSx::proto::eAsk>(what))
	{
	default:
		logger->log ("WARN    RSProto RCV unkwnown msg [what=0x%04X] [payloadLen=%d]\n", what, payloadLen);
		break;

	case rhFSx::proto::eAsk::rsproto_service_cmd:
		//ho ricevuto un generico comando da RSProto. Nel payload c'è un ID che identifica il comando stesso
		logger->log ("RSProto RCV [0x%04X - rsproto_service_cmd] [payloadLen=%d]\n", what, payloadLen);
		if (payloadLen >= 4)
		{
			const u32 cmd = rhea::utils::bufferReadU32(payload);
			switch (cmd)
			{
			case 0x8000:
				//RSProto mi informa sullo stato della connessione verso il cloud
				logger->log ("RSProto RCV [0x%04X - rsproto_service_cmd] [cmd=0x%04X] [payloadLen=%d]\n", what, cmd, payloadLen);
				{
					const u8 connectionIsUpWithSECO = payload[4];
					if (connectionIsUpWithSECO)
					{
						priv_sendLastSMUState(logger);

						//forzo l'invio delle temp al prossimo giro
						priv_reset();
					}
				}
				break;
			}
		}		
		break;

	case rhFSx::proto::eAsk::rsproto_file:
		//ho ricevuto un file da RSProto, oppure una richiesta di inviare un file a RSProto
		if (payloadLen < 4)
		{
			logger->log ("WARN  RSProto rcv invalid payloadLen. RCV command 'rsproto_file' [payloadLen=%d]\n", payloadLen);
		}
		else
		{
			const u32	fileID = rhea::utils::bufferReadU32 (payload);
				
			if (payloadLen == 4)
				priv_RSProtoWantsAFile (server, fileID, userValue, logger);
			else
			{
				//RSProto mi sta comunicando che ha ricevuto un file dal cloud e mi ha quindi passato un fileID e un path assoluto.
				//Da ora in poi, il file è "mio", la responsabilità di eliminarlo è nelle mie mani
				const u8 *fileFullPath = &payload[4];
				priv_RSProtoSentMeAFile (server, fileID, fileFullPath ,logger);
			}
		}
		break;
	}
}

//******************************************* 
bool RSProto::priv_copyFileInAutoupdateFolder (const u8 *fullSrcFilePathAndName) const
{
	u8 s[512];
    rhea::string::utf8::spf (s, sizeof(s), "%s/autoUpdate", rhea::getPhysicalPathToAppFolder());
    return rhea::fs::fileCopyAndKeepSameName (fullSrcFilePathAndName, s);
}


//******************************************* 
void RSProto::priv_RSProtoSentMeAFile (cpubridge::Server *server, u32 fileID, const u8 *fileFullPath, rhea::ISimpleLogger *logger)
{
	//ho ricevuto un file da RSProto
	logger->log ("RSProto sent me a file [id:0x%04X] [path:%s]\n", fileID, fileFullPath);
	logger->incIndent();

	switch (fileID)
	{
	default:
		logger->log ("invalid file ID\n");
		break;

	case FILE_EVA_DTS:
		//non me ne faccio nulla di questo file, non dovrebbe nemmeno mai arrivare
		logger->log ("it's an eva-dts, doing nothing with it\n");
		break;

	case FILE_MACHINE_CONFIG:
		//Mi è arrivato un file di configurazione macchina, lo metto nella cartella dell'auto aggiornamento e schedulo un reboot
		logger->log ("it's a FILE_MACHINE_CONFIG, copying in autoupdate folder and scheduling a reboot ASAP\n");
        if (priv_copyFileInAutoupdateFolder (fileFullPath))
            server->scheduleAction_relaxedReboot();
		break;

	case FILE_SMU:
		logger->log ("it's a FILE_SMU, copying in autoupdate folder and scheduling a reboot ASAP\n");
        if (priv_copyFileInAutoupdateFolder (fileFullPath))
            server->scheduleAction_relaxedReboot();
        break;

	case FILE_CPU:
		logger->log ("it's a FILE_CPU, copying in autoupdate folder and scheduling a reboot ASAP\n");
        if (priv_copyFileInAutoupdateFolder (fileFullPath))
            server->scheduleAction_relaxedReboot();
		break;

	case FILE_GUI:
		logger->log ("it's a FILE_GUI, copying in autoupdate folder and scheduling a reboot ASAP\n");
        //if (priv_copyFileInAutoupdateFolder (fileFullPath))
        //	server->scheduleAction_relaxedReboot();
		break;
	}

	//elimino il file dalla cartella di RSProto
	logger->log ("deleting file %s\n", fileFullPath);
	rhea::fs::fileDelete (fileFullPath);
	logger->decIndent();
}

//******************************************* 
void RSProto::priv_RSProtoWantsAFile (cpubridge::Server *server UNUSED_PARAM, u32 fileID, u16 userValue, rhea::ISimpleLogger *logger)
{
	//RSProto vuole che io gli mandi il file [fileID]
	logger->log ("RSProto wants file [0x%04X]\n", fileID);
	logger->incIndent();

	//La risposta attesa da RSProto è:
		//0x9002
		//userValue	(16bit)
		//fileID (32bit)
		//full file path and name

	u8	fullFilePathAndName[256];
	eAnswerToFileRequest answer = eAnswerToFileRequest::notAvail;
	switch (fileID)
	{
	default:
		//errore, non dovrebbe accadere. In ogni caso, rispondo che non ho il file
		logger->log ("ERR: unknown file ID, answering 'file not avail'\n");
		answer = eAnswerToFileRequest::notAvail;
		break;

	case FILE_EVA_DTS:
		logger->log ("FILE_EVA_DTS: answering wait\n");
		answer = eAnswerToFileRequest::wait;

		//il file lo devo prima ottenere da CPU, quindi schedulo un'azione e per il momento non rispondo nulla a RSProto
        server->scheduleAction_downloadEVADTSAndAnswerToRSProto();
		break;

	case FILE_MACHINE_CONFIG:
		logger->log ("FILE_MACHINE_CONFIG: ");
		rhea::string::utf8::spf (fullFilePathAndName, sizeof(fullFilePathAndName), "%s/current/da3/vmcDataFile.da3", rhea::getPhysicalPathToAppFolder());
		logger->log ("sending file %s\n", fullFilePathAndName);
		answer = eAnswerToFileRequest::sendFile;
		break;

	case FILE_SMU:
		answer = eAnswerToFileRequest::notAvail;
		break;

	case FILE_CPU:
		answer = eAnswerToFileRequest::notAvail;
		break;

	case FILE_GUI:
		answer = eAnswerToFileRequest::notAvail;
		break;
	}

	switch (answer)
	{
	default:
	case eAnswerToFileRequest::notAvail:
		sendFileAnswer (fileID, userValue, NULL);
		break;

	case eAnswerToFileRequest::wait:
		break;

	case eAnswerToFileRequest::sendFile:
		sendFileAnswer (fileID, userValue, fullFilePathAndName);
		break;
	}
	logger->decIndent();
}

//******************************************* 
void RSProto::sendFileAnswer (u32 fileID, u16 userValue, const u8 *fullPathName)
{
	//Rispondo:
	//0x9002
	//userValue
	//id file su 32 bit
	//nome del file full path

	u8 bufferW[256];
	rhea::utils::bufferWriteU32 (bufferW, fileID);

	if (NULL == fullPathName)
	{
		//rispondo "not avail"
		bufferW[4] = 0x00;
		priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_file), userValue, bufferW, 5);
	}
	else
	{
		const u32 filenameSize = rhea::string::utf8::lengthInBytes(fullPathName);
		if (4 + filenameSize < sizeof(bufferW))
		{
			memcpy (&bufferW[4], fullPathName, filenameSize);
			priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_file), userValue, bufferW, 4 + filenameSize);
		}
		else
		{
			DBGBREAK;
			bufferW[4] = 0x00;
			priv_send (static_cast<u16>(rhFSx::proto::eAsk::rsproto_file), userValue, bufferW, 5);
		}
	}
}
