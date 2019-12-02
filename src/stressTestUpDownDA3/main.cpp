#include <conio.h>
#include "stress.h"

#define DEFAULT_PORT_NUMBER			2280

socketbridge::SokBridgeClientVer	version;
socketbridge::SokBridgeIDCode		idCode;
u32									SMUVersion = 0;
bool								bQuitMainThread;


struct sThreadInitParam
{
	HThreadMsgR handleR;
	rhea::ISimpleLogger *wt;
};


//*****************************************************
void handleDecodedMsg (const rhea::app::sDecodedEventMsg &decoded, rhea::ISimpleLogger *log)
{
	switch (decoded.eventType)
	{
	default:
		log->log("UNHANDLED event [0x%02X], payloadLen [%d]\n", decoded.eventType, decoded.payloadLen);
		break;

	case socketbridge::eEventType_cpuMessage:
		/*{
			u8 msgImportanceLevel;
			u16 msgLenInBytes;
			u8 msgUTF8[96];
			rhea::app::CurrentCPUMessage::decodeAnswer (decoded, &msgImportanceLevel, &msgLenInBytes, msgUTF8, sizeof(msgUTF8));

			if (msgUTF8[0] != 0x00)
				log->log("RCV [cpuMessage] => impLvl[%d], msgLen[%d], msg[%s]", msgImportanceLevel, msgLenInBytes, msgUTF8);
			else
				log->log("RCV [cpuMessage] => impLvl[%d], msgLen[%d]", msgImportanceLevel, msgLenInBytes);
			log->log("\n");
		}*/
		break;

	case socketbridge::eEventType_creditUpdated:
		{
			char credit[16];
			rhea::app::CurrentCredit::decodeAnswer(decoded, (u8*)&credit, sizeof(credit));
			log->log("RCV [creditChange] => [%s]\n", credit);
		}
		break;

	case socketbridge::eEventType_selectionRequestStatus:
		{
			cpubridge::eRunningSelStatus runningSelStatus;
			rhea::app::CurrentSelectionRunningStatus::decodeAnswer(decoded, &runningSelStatus);
			log->log("RCV [selectionRequestStatus] => %s [%d]\n", rhea::app::utils::verbose_eRunningSelStatus(runningSelStatus), (u8)runningSelStatus);
		}
		break;

	case socketbridge::eEventType_cpuStatus:
		{
			cpubridge::eVMCState vmcState;
			u8 vmcErrorCode, vmcErrorType;
			u16 flag1 = 0;

			rhea::app::CurrentCPUStatus::decodeAnswer (decoded, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);
			log->log("RCV [cpuStatus] => state=[%d %s], err_code=[%d], err_type=[%d] flag[%04X]\n", vmcState, rhea::app::utils::verbose_eVMCState(vmcState), vmcErrorCode, vmcErrorType, flag1);
		}
		break;

	case socketbridge::eEventType_reqClientList:
		{
			log->log("RCV [reqClientList]\n");
			log->incIndent();
			rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

			rhea::DateTime dtCPUBridgeStarted;
			socketbridge::sIdentifiedClientInfo *list = NULL;
			u16 nClientConnected = 0;
			u16 nClientInfo = 0;
			rhea::app::ClientList::decodeAnswer (decoded, allocator, &nClientConnected, &nClientInfo, &dtCPUBridgeStarted, &list);

			log->log("Num connected client: %d\n", nClientConnected);
			for (u32 i = 0; i < nClientInfo; i++)
			{
				char sVerInfo[64];
				if (list[i].clientVer.apiVersion == 0x01 && list[i].clientVer.appType == 0x02 && list[i].clientVer.unused2 == 0x03 && list[i].clientVer.unused3 == 0x04 &&
					list[i].idCode.data.buffer[0] == 0x05 && list[i].idCode.data.buffer[1] == 0 && list[i].idCode.data.buffer[2] == 0 && list[i].idCode.data.buffer[3] == 0)
				{
					//caso speciale delle prime GU FUsione beta v1, s'ha da rimuovere prima o poi
					sprintf_s(sVerInfo, sizeof(sVerInfo), "old-GUI_beta-v1");
				}
				else
					rhea::app::utils::verbose_SokBridgeClientVer(list[i].clientVer, sVerInfo, sizeof(sVerInfo));

				log->log("-----------------------------------------------------client #%02d\n", (i + 1));
				log->log("idCode: 0x%08X\n", list[i].idCode.data.asU32);
				log->log("ver: %s\n", sVerInfo);
				if (list[i].currentWebSocketHandleAsU32 == u32MAX)
					log->log("socket bound: no\n");
				else
					log->log("socket bound: yes\n");

				char s[32];
				rhea::DateTime dt;
				dt = dtCPUBridgeStarted;
				dt.addMSec(list[i].timeCreatedMSec);
				dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '-', ':');
				log->log("timeCreated: %s\n", s);

				dt = dtCPUBridgeStarted;
				dt.addMSec(list[i].lastTimeRcvMSec);
				dt.formatAs_YYYYMMDDHHMMSS(s, sizeof(s), ' ', '-', ':');
				log->log("lastTimeRCV: %s\n", s);
			}
			log->decIndent();

			if (list)
				RHEAFREE(allocator, list);
		}
		break;

	case socketbridge::eEventType_selectionAvailabilityUpdated:
		{
			u8 numSel = 0;
			u8 selAvailability[128];
			rhea::app::CurrentSelectionAvailability::decodeAnswer(decoded, &numSel, selAvailability, sizeof(selAvailability));
			log->log("RCV [selAvailability]\n");
			log->incIndent();
			log->log("Num sel=[%d]\n", numSel);
			u8 i = 0;
			while (i < numSel)
			{
				char s[32];
				memset(s, 0, sizeof(s));
				u8 i2 = 12; //12 per riga
				while (i < numSel && i2)
				{
					if (selAvailability[i] == 0)
						strcat(s, "0 ");
					else
						strcat(s, "1 ");
					--i2;
					++i;
				}
				log->log("%s\n", s);
			}
			log->decIndent();
		}
		break;

	case socketbridge::eEventType_btnProgPressed:
		rhea::app::ButtonProgPressed::decodeAnswer(decoded);
		log->log("RCV [btn prog pressed]\n");
		break;

	case socketbridge::eEventType_reqDataAudit:
		{
			cpubridge::eReadDataFileStatus status;
			u16 toKbSoFar;
			u16 fileID;
			rhea::app::ReadDataAudit::decodeAnswer(decoded, &status, &toKbSoFar, &fileID);

			log->log("readDataAudit: status[%s] totKbSoFar[%d] fileID[%d]\n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);
		}
		break;

	case socketbridge::eEventType_reqIniParam:
		{
			cpubridge::sCPUParamIniziali iniParam;
			rhea::app::CurrentCPUInitParam::decodeAnswer(decoded, &iniParam);
			log->log("RCV [iniParam]: CPU_ver[%s] protocol_ver[%d]\n", iniParam.CPU_version, iniParam.protocol_version);
		}
		break;

	case socketbridge::eEventType_reqVMCDataFile:
		{
			cpubridge::eReadDataFileStatus status;
			u16 toKbSoFar;
			u16 fileID;
			rhea::app::ReadVMCDataFile::decodeAnswer(decoded, &status, &toKbSoFar, &fileID);
			log->log("readVMCDataFile: status[%s] totKbSoFar[%d] fileID[%d]\n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);
		}
		break;

	case socketbridge::eEventType_reqVMCDataFileTimestamp:
		{
			char text[128];
			sprintf_s(text, sizeof(text), "da3 timestamp:");

			cpubridge::sCPUVMCDataFileTimeStamp ts;
			rhea::app::CurrentVMCDataFileTimestamp::decodeAnswer(decoded, &ts);
			u8 buffer[16];
			ts.writeToBuffer(buffer);
			for (u8 i = 0; i < ts.getLenInBytes(); i++)
			{
				char s[4];
				rhea::string::format::Hex8(buffer[i], s, sizeof(s));

				char ss[16];
				sprintf_s (ss, sizeof(ss), " [%s]",s);
				strcat_s(text, sizeof(text), ss);
			}
			strcat_s(text, sizeof(text), "\n");
			log->log(text);
		}
		break;

	case socketbridge::eEventType_reqWriteLocalVMCDataFile:
		{
			cpubridge::eWriteDataFileStatus status;
			u16 toKbSoFar;
			rhea::app::WriteLocalVMCDataFile::decodeAnswer(decoded, &status, &toKbSoFar);
			log->log("WriteLocalVMCDataFile: status[%s] totKbSoFar[%d]\n", rhea::app::utils::verbose_writeDataFileStatus(status), toKbSoFar);
		}
		break;

	case socketbridge::eEventType_cpuSanWashingStatus:
		{
			u8 b0, b1, b2;
			rhea::app::SanWashingStatus::decodeAnswer (decoded, &b0, &b1, &b2);
			log->log("SanWashingStatus: fase[%d] btn1[%d] btn2[%d]\n", b0, b1, b2);
		}
		break;

	case socketbridge::eEventType_cpuWritePartialVMCDataFile:
		{
			u8 blockWritten = 0;
			rhea::app::WritePartialVMCDataFile::decodeAnswer(decoded, &blockWritten);
			log->log("PArtial DA3 written: block[%d]\n", blockWritten);
		}
		break;

	case socketbridge::eEventType_cpuExtendedConfigInfo:
		{
			cpubridge::sExtendedCPUInfo info;			
			rhea::app::ExtendedConfigInfo::decodeAnswer(decoded, &info);
			log->log("cpuExtendedConfigInfo: ver[%d] type[%s] model[%d] induzione?[%d]\n", info.msgVersion, rhea::app::utils::verbose_CPUMachineType(info.machineType), info.machineModel, info.isInduzione);
		}
		break;

	case socketbridge::eEventType_getAllDecounters:
		{
			u16 valori[32];
			rhea::app::GetAllDecounters::decodeAnswer(decoded, valori);

			char s[256];
			sprintf_s(s, sizeof(s), "eEventType_getAllDecounters: %d", valori[0]);
			for (u8 i = 1; i < 13; i++)
			{
				char ss[16];
				sprintf_s(ss, sizeof(ss), " %d", valori[i]);
				strcat_s(s, sizeof(s), ss);
			}
			log->log (s);
		}
		break;		

	case socketbridge::eEventType_setDecounter:
		{
			cpubridge::eCPUProgrammingCommand_decounter which;
			u16 value = 0;
			rhea::app::SetDecounter::decodeAnswer(decoded, &which, &value);
			log->log("setDecounter: which[%d] value[%d]\n", (u8)which, value);
		}
		break;		
	}
}

//*****************************************************
void handleMsgFromSocketServer (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, rhea::app::FileTransfer *ftransf, rhea::ISimpleLogger *log)
{
	while (1)
	{
		u16 nRead = proto->read(ch, 100, bufferR);
		if (nRead == 0)
			return;
		if (nRead >= rhea::protocol::RES_ERROR)
			return;

		rhea::app::sDecodedMsg decoded;
		u8 *buffer = bufferR._getPointer(0);
		u16 nBytesUsed = 0;
		while (rhea::app::decodeSokBridgeMessage(buffer, nRead, &decoded, &nBytesUsed))
		{
			switch (decoded.what)
			{
			case rhea::app::eDecodedMsgType_event:
				handleDecodedMsg(decoded.data.asEvent, log);
				break;

			case rhea::app::eDecodedMsgType_fileTransf:
				ftransf->onMessage(rhea::getTimeNowMSec(), decoded.data.asFileTransf);
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

//*****************************************************
bool identify (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, rhea::ISimpleLogger *log)
{
	if (!rhea::app::handleInitialRegistrationToSocketBridge(log, ch, proto, bufferR, version, &idCode, &SMUVersion))
	{
		//chiudo
		log->log("closing connection\n");
		proto->close(ch);
		return false;
	}
	
	log->log("--------------------------------------\nWe are online!!\n--------------------------------------\n");
	return true;
}


//*****************************************************
bool connect (rhea::ProtocolChSocketTCP *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, const char *IP, u16 PORT_NUMBER, rhea::ISimpleLogger *logger)
{
	logger->log("connecting to [%s]:[%d]\n", IP, PORT_NUMBER);
	logger->incIndent();

	//apro una socket
	OSSocket sok;
	eSocketError err = OSSocket_openAsTCPClient (&sok, IP, PORT_NUMBER);
	if (eSocketError_none != err)
	{
		logger->log("FAILED with errCode=%d\n", (u32)err);
		logger->decIndent();
		return false;
	}

	//bindo la socket al channel
	logger->log("connected\n");
	ch->bindSocket(sok);

	//handshake
	logger->log("sending handshake to SMU\n");
	logger->incIndent();
	if (!proto->handshake_clientSend(ch, logger))
	{
		logger->log("FAIL\n");
		logger->decIndent();
		logger->decIndent();
		return false;
	}

	//siamo connessi
	logger->log("handshake OK\n");
	logger->decIndent();

	//identificazione
	if (identify (ch, proto, bufferR, logger))
	{
		logger->decIndent();
		return true;
	}

	logger->decIndent();
	return false;
}

//*****************************************************
void disconnect (rhea::ProtocolChSocketTCP *ch, OSWaitableGrp *waitGrp, rhea::ISimpleLogger *logger)
{
	if (ch->isOpen())
	{
		logger->log("disconnecting\n");
		waitGrp->removeSocket(ch->getSocket());
		ch->close();
	}
}

/*****************************************************
 * true se il main deve aspettare per un char prima di terminare
 * false se il main termina all'istante
 */
bool run (const sThreadInitParam *init)
{
	//parametri di init
	HThreadMsgR msgQHandleR = init->handleR;
	rhea::ISimpleLogger *logger = init->wt;

	//allocatore locale
	rhea::Allocator	*localAllocator = rhea::memory_getDefaultAllocator();

	//canale di comunicazione
	rhea::ProtocolChSocketTCP ch(localAllocator, 4096, 8192);

	//Protocollo 
	rhea::ProtocolConsole proto(localAllocator, 1024, 4096);

	//creazione del buffer di ricezione
	rhea::LinearBuffer	bufferR;
	bufferR.setup(localAllocator, 8192);

	//handler dei file transfer
	rhea::app::FileTransfer fileTransferManager;
	fileTransferManager.setup(localAllocator, logger);

	//Setup della waitableGrp
	OSWaitableGrp waitGrp;
	{
		OSEvent hMsgQEvent;
		rhea::thread::getMsgQEvent (msgQHandleR, &hMsgQEvent);
		waitGrp.addEvent(hMsgQEvent, u32MAX);
	}


	//connessione automatica a localhost
	if (connect(&ch, &proto, bufferR, "127.0.0.1", DEFAULT_PORT_NUMBER, logger))
		waitGrp.addSocket(ch.getSocket(), u32MAX);


	Stress stress;
	stress.setup(msgQHandleR, &proto, &ch, &bufferR, &waitGrp, logger);


	const char da3_filename[] = { "C:/rhea/rheaSRC/gpu-fts-nestle-2019/src/stressTestUpDownDA3/esempio.da3" };
	u32 da3_sizeOfBuffer = 0;
	u8 *da3_buffer = rhea::fs::fileCopyInMemory(da3_filename, localAllocator, &da3_sizeOfBuffer);
	
	u32 numTotTentativi = 0;
	u32 numFallimenti = 0;
	while (1)
	{
		char s[64];
		sprintf_s(s, sizeof(s), "N=%d, falliti=%d\n", numTotTentativi, numFallimenti);
		logger->log(s);

		//uppo il file sulla CPU
		if (stress.uploadDA3(da3_filename))
		{
			//chiedo il DA3 alla cpu
			u16 fileID = stress.downloadDA3();

			if (fileID != u16MAX)
			{
				numTotTentativi++;

				char downloadedFilename[256];
				sprintf_s(downloadedFilename, sizeof(downloadedFilename), "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/temp/vmcDataFile%d.da3", fileID);

				char currentFilename[256];
				sprintf_s(currentFilename, sizeof(currentFilename), "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/current/da3/vmcDataFile.da3");

				//confronto il file attuale in current/da3 con quello appena scaricato
				u32 sizeOfBuffer1 = 0;
				u8 *buffer1 = rhea::fs::fileCopyInMemory(currentFilename, localAllocator, &sizeOfBuffer1);

				u32 sizeOfBuffer2 = 0;
				u8 *buffer2 = rhea::fs::fileCopyInMemory(downloadedFilename, localAllocator, &sizeOfBuffer2);

				bool bEqual = true;
				if (sizeOfBuffer1 != sizeOfBuffer2)
					bEqual = false;
				else
				{
					//buffer[9693] = da3_buffer[9693];
					//buffer[9705] = da3_buffer[9705];
					if (memcmp(buffer1, buffer2, 156*64) != 0)
						bEqual = false;
				}
				RHEAFREE(localAllocator, buffer1);
				RHEAFREE(localAllocator, buffer2);

				if (!bEqual)
				{
					numFallimenti++;
					printf("fallito\n");
					break;
				}


				//elimino quello appenda downloadato
				rhea::fs::fileDelete(downloadedFilename);
			}

			//aspetto un po'
			rhea::thread::sleepMSec(5000);
		}
	}

	





	//unsetup
	fileTransferManager.unsetup();
	bufferR.unsetup();

	//chiudo
	logger->log("closing connection\n");
	proto.close(&ch);
	return false;
}


//*****************************************************
i16 threadFn (void *userParam)
{
	sThreadInitParam *init = (sThreadInitParam*)userParam;
	run (init);
	return 0;
}




//*****************************************************
void go()
{
	//creo una msgQ per comunicare con il thread
	HThreadMsgR handleR;
	HThreadMsgW handleW;
	rhea::thread::createMsgQ(&handleR, &handleW);


	//console setup
	rhea::StdoutLogger wt;

	//listening thread 
	sThreadInitParam initParam;
	initParam.handleR = handleR;
	initParam.wt = &wt;

	rhea::HThread hThread;
	rhea::thread::create(&hThread, threadFn, &initParam);

	rhea::thread::waitEnd(hThread);
	rhea::thread::deleteMsgQ(handleR, handleW);
}

//*****************************************************
int main()
{
	HINSTANCE hInst = NULL;
	rhea::init("stressTestUpDownDA3", &hInst);

	//version info
	version.apiVersion = 0x01;
	version.appType = socketbridge::SokBridgeClientVer::APP_TYPE_CONSOLE;
	version.unused2 = 0x00;
	version.unused3 = 0x00;

	//idCode della connessione a SocketBridge
	memset(&idCode, 0, sizeof(idCode));

	go();
	
    rhea::deinit();
	return 0;
}


