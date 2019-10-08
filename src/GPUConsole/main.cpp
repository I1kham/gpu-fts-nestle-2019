#include <conio.h>
#include "../rheaAppLib/rheaApp.h"
#include "../rheaAppLib/rheaAppUtils.h"
#include "../rheaAppLib/rheaAppFileTransfer.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaCommonLib/Protocol/ProtocolChSocketTCP.h"
#include "../rheaCommonLib/Protocol/ProtocolConsole.h"
#include "Terminal.h"
#include "command/UserCommand.h"

#define DEFAULT_PORT_NUMBER			2280

socketbridge::SokBridgeClientVer	version;
socketbridge::SokBridgeIDCode		idCode;
UserCommandFactory					userCommandFactory;
u32									SMUVersion = 0;
bool								bQuitMainThread;


struct sThreadInitParam
{
	HThreadMsgR handleR;
	Terminal   *wt;
};


//*****************************************************
void update_console_header(WinTerminal *wt)
{
	char s[256];
	sprintf(s, "rheaConsole [ver 0x%02X.0x%02X.0x%02X.0x%02X] [idCode 0x%08X] [SMUVer 0x%02X.0x%02X]",
		version.apiVersion, version.appType, version.unused2, version.unused3, 
		idCode.data.asU32, 
		(SMUVersion & 0x0000FF00)>>8, (SMUVersion & 0x000000FF));
	wt->setHeader(s);
}


/*****************************************************
 *	connect [ip] [port]
 *
 *	se ip no è specificato, assume di default IP=127.0.0.1 e PORT=DEFAULT_PORT_NUMBER
 *	se ip è specificato e port non lo è, assume PORT=DEFAULT_PORT_NUMBER
 */
void handleCommandSyntax_connect (const char *s, char *out_ip, u32 sizeofIP, u16 *out_port)
{
	sprintf_s (out_ip, sizeofIP, "127.0.0.1");
	*out_port = DEFAULT_PORT_NUMBER;

	rhea::string::parser::Iter iter;
	rhea::string::parser::Iter iter2;
	
	iter.setup(s);
	rhea::string::parser::advanceUntil(iter, " ", 1);
	rhea::string::parser::toNextValidChar(iter);

	//se c'è un ip
	if (rhea::string::parser::extractValue(iter, &iter2, " ", 1))
	{
		iter2.copyCurStr(out_ip, sizeofIP);

		rhea::string::parser::toNextValidChar(iter);

		i32 port;
		if (rhea::string::parser::extractInteger(iter, &port))
		{
			if (port > 0 && port < 65536)
				*out_port = (u16)port;
		}
	}

}

//*****************************************************
void handleCommandSyntax_help (WinTerminal *logger)
{
	logger->log("Console command list:\n");
	logger->incIndent();
		logger->log("cls\n");
		logger->log("connect [ip] | [port]\n");
		logger->log("disconnect\n");
		logger->log("exit\n");
		userCommandFactory.help_commandLlist(logger);
	logger->decIndent();
}

//*****************************************************
void handleDecodedMsg (const rhea::app::sDecodedEventMsg &decoded, WinTerminal *log)
{
	switch (decoded.eventType)
	{
	default:
		log->outText(true, false, false, "UNHANDLED event [0x%02X], payloadLen [%d]\n", decoded.eventType, decoded.payloadLen);
		break;

	case socketbridge::eEventType_cpuMessage:
		{
			u8 msgImportanceLevel;
			u16 msgLenInBytes;
			u8 msgUTF8[96];
			rhea::app::CurrentCPUMessage::decodeAnswer (decoded, &msgImportanceLevel, &msgLenInBytes, msgUTF8, sizeof(msgUTF8));

			if (msgUTF8[0] != 0x00)
				log->outText(true, true, false,"RCV [cpuMessage] => impLvl[%d], msgLen[%d], msg[%s]", msgImportanceLevel, msgLenInBytes, msgUTF8);
			else
				log->outText(true, true, false, "RCV [cpuMessage] => impLvl[%d], msgLen[%d]", msgImportanceLevel, msgLenInBytes);
			log->log("\n");
		}
		break;

	case socketbridge::eEventType_selectionRequestStatus:
		{
			cpubridge::eRunningSelStatus runningSelStatus;
			rhea::app::CurrentSelectionRunningStatus::decodeAnswer(decoded, &runningSelStatus);
			log->outText(true, true, false, "RCV [selectionRequestStatus] => %s [%d]\n", rhea::app::utils::verbose_eRunningSelStatus(runningSelStatus), (u8)runningSelStatus);
		}
		break;

	case socketbridge::eEventType_cpuStatus:
		{
			cpubridge::eVMCState vmcState;
			u8 vmcErrorCode, vmcErrorType;
			rhea::app::CurrentCPUStatus::decodeAnswer (decoded, &vmcState, &vmcErrorCode, &vmcErrorType);
			log->outText (true, true, false, "RCV [cpuStatus] => state=[%d %s], err_code=[%d], err_type=[%d]\n", vmcState, rhea::app::utils::verbose_eVMCState(vmcState), vmcErrorCode, vmcErrorType);
		}
		break;

	case socketbridge::eEventType_reqClientList:
		{
			log->outText(true, true, false, "RCV [reqClientList]\n");
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
			log->outText(true, true, false, "RCV [selAvailability]\n");
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
	}
}

//*****************************************************
void handleMsgFromSocketServer (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, rhea::app::FileTransfer *ftransf, WinTerminal *log)
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

/*****************************************************
 * in generale ritorn 0 ma per alcuni msg specifici ritorna > 0
 * Ritorna:
 *	0xff => comando non riconosciuto
 *	0x01 => se il comando è "connect"
 *	0x02 => se il comando è "disconnect"
 */
u8 handleUserInput (const char *s, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf)
{
	if (strncmp(s, "connect", 7) == 0)
		return 0x01;

	if (strncmp(s, "help", 4) == 0)
	{
		handleCommandSyntax_help(log);
		return 0;
	}

	if (!ch->isOpen())
	{
		log->outText(true, false, false, "not connected\n");
		return 0;
	}

	if (strcasecmp(s, "disconnect") == 0)
		return 0x02;


	if (userCommandFactory.handle(s, ch, proto, log, ftransf))
		return 0;

	if (strcasecmp(s, "upload1") == 0)
	{
		log->log("sending [%s]...\n", s);
		log->incIndent();

		rhea::app::FileTransfer::Handle handle;
		if (ftransf->startFileUpload(ch, proto, rhea::getTimeNowMSec(), "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/test_upload_small_file.gif", "test", &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
		log->decIndent();
		return 0;
	}
	if (strcasecmp(s, "upload2") == 0)
	{
		log->log("sending [%s]...\n", s);
		log->incIndent();

		rhea::app::FileTransfer::Handle handle;
		if (ftransf->startFileUpload(ch, proto, rhea::getTimeNowMSec(), "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/test_upload_big_file.gif", "test", &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
		log->decIndent();
		return 0;
	}

	if (strcasecmp(s, "download-test") == 0)
	{
		log->log("sending [%s]...\n", s);
		log->incIndent();

		rhea::app::FileTransfer::Handle handle;
		if (ftransf->startFileDownload(ch, proto, rhea::getTimeNowMSec(), "test", "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/file_downloadata_da_smu", &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
		log->decIndent();
		return 0;
	}

	log->outText(true,false,false,"unknown command [%s]\n", s);
	return 0xff;

}




//*****************************************************
bool identify (rhea::IProtocolChannell *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, WinTerminal *log)
{
	if (!rhea::app::handleInitialRegistrationToSocketBridge(log, ch, proto, bufferR, version, &idCode, &SMUVersion))
	{
		//chiudo
		log->log("closing connection\n");
		proto->close(ch);
		return false;
	}
	
	log->log("--------------------------------------\nWe are online!!\n--------------------------------------\n");
	update_console_header(log);

	return true;
}


//*****************************************************
bool connect (rhea::ProtocolChSocketTCP *ch, rhea::IProtocol *proto, rhea::LinearBuffer &bufferR, const char *IP, u16 PORT_NUMBER, WinTerminal *logger)
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
void disconnect (rhea::ProtocolChSocketTCP *ch, OSWaitableGrp *waitGrp, WinTerminal *logger)
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
	WinTerminal *logger = init->wt;

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


	//setup della lista dei comandi utenti riconosciuti
	userCommandFactory.setup(localAllocator);
	userCommandFactory.utils_addAllKnownCommands();


	//connessione automatica a localhost
	if (connect(&ch, &proto, bufferR, "127.0.0.1", DEFAULT_PORT_NUMBER, logger))
		waitGrp.addSocket(ch.getSocket(), u32MAX);

	//main loop
	bool bQuit = false;
	while (bQuit == false)
	{
		//ogni 10 secondi mi sblocco indipendentemente dall'avere ricevuto notifiche o meno
		u8 nEvent = waitGrp.wait(10000);

		//vediamo cosa mi ha svegliato
		for (u8 i = 0; i < nEvent; i++)
		{
			if (OSWaitableGrp::evt_origin_socket == waitGrp.getEventOrigin(i))
			{
				//ho qualcosa sulla socket
				handleMsgFromSocketServer(&ch, &proto, bufferR, &fileTransferManager, logger);
			}
			else if (OSWaitableGrp::evt_origin_osevent == waitGrp.getEventOrigin(i))
			{
				//ho qualcosa sulla msgQ di questo thread
				rhea::thread::sMsg msg;
				while (rhea::thread::popMsg(msgQHandleR, &msg))
				{
					switch (msg.what)
					{
					default:
						DBGBREAK;
						break;

					case MSGQ_DIE:
						bQuit = true;
						break;

					case MSGQ_USER_INPUT:
						switch (handleUserInput((const char*)msg.buffer, &ch, &proto, logger, &fileTransferManager))
						{
						default:
							break;

						case 0x01: //connect
							{
								disconnect(&ch, &waitGrp, logger);
								
								char ip[64];
								u16 port;
								handleCommandSyntax_connect((const char*)msg.buffer, ip, sizeof(ip), &port);
								if (connect(&ch, &proto, bufferR, ip, port, logger))
								{
									waitGrp.addSocket(ch.getSocket(), u32MAX);
								}
							}
							break;

						case 0x02: //disconnect
							disconnect(&ch, &waitGrp, logger);
							break;
						}
						break;
					}
					rhea::thread::deleteMsg(msg);
				}
			}
		}

		//update del file transfer manager
		u64 timeNowMSec = rhea::getTimeNowMSec();
		if (fileTransferManager.update(timeNowMSec))
		{
			rhea::app::FileTransfer::sTansferInfo info;
			while (fileTransferManager.popEvent(&info))
			{
				u32 handle = info.handle.asU32();
				const f32 timeElapsedSec = (f32)info.timeElapsedMSec / 1000.0f;
				f32 KbSec = ((f32)info.currentTransferSizeInBytes / timeElapsedSec) / 1024.0f;

				logger->log("FTE => handle[0x%08X] transferred[%d/%d], speed[%.2f] kB/s, status[%d = %s], fail[%d = %s]\n",
					handle,
					info.currentTransferSizeInBytes, info.totalTransferSizeInBytes,
					KbSec,
					info.status, rhea::app::utils::verbose_fileTransferStatus(info.status),
					info.failReason, rhea::app::utils::verbose_fileTransferFailReason(info.failReason)
				);
			}
		}

	}

	//unsetup
	fileTransferManager.unsetup();
	bufferR.unsetup();
	userCommandFactory.unsetup();

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
	Terminal wt(handleW);
	wt.setup();

	update_console_header(&wt);

	//listening thread 
	sThreadInitParam initParam;
	initParam.handleR = handleR;
	initParam.wt = &wt;

	rhea::HThread hThread;
	rhea::thread::create(&hThread, threadFn, &initParam);


	wt.loop();
	
	rhea::thread::pushMsg (handleW, MSGQ_DIE, (u32)0);
	rhea::thread::waitEnd(hThread);
	rhea::thread::deleteMsgQ(handleR, handleW);
}

//*****************************************************
int main()
{
	HINSTANCE hInst = NULL;
	rhea::init("rheaConsole", &hInst);

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


