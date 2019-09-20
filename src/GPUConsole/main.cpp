#include <conio.h>
#include "../rheaAppLib/rheaApp.h"
#include "../rheaAppLib/rheaAppUtils.h"
#include "../rheaAppLib/rheaAppFileTransfer.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "../rheaCommonLib/Protocol/ProtocolChSocketTCP.h"
#include "../rheaCommonLib/Protocol/ProtocolConsole.h"
#include "Terminal.h"

socketbridge::SokBridgeClientVer	version;
socketbridge::SokBridgeIDCode		idCode;
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
		u16 nUsed = rhea::app::decodeSokBridgeMessage(bufferR._getPointer(0), nRead, &decoded);
		if (nUsed)
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
		}
	}
}

//*****************************************************
void handleUserInput(const char *s, rhea::IProtocolChannell *ch, rhea::IProtocol *proto, WinTerminal *log, rhea::app::FileTransfer *ftransf)
{
	if (strcasecmp(s, "list") == 0)
	{
		log->log("sending [%s]...\n", s);
		rhea::app::ClientList::ask(ch, proto);
		return;
	}
	if (strcasecmp(s, "cpumsg") == 0)
	{
		log->log("sending [%s]...\n", s);
		rhea::app::CurrentCPUMessage::ask(ch, proto);
		return;
	}
	if (strcasecmp(s, "cpustatus") == 0)
	{
		log->log("sending [%s]...\n", s);
		rhea::app::CurrentCPUStatus::ask(ch, proto);
		return;
	}
	if (strcasecmp(s, "selavail") == 0)
	{
		log->log("sending [%s]...\n", s);
		rhea::app::CurrentSelectionAvailability::ask(ch, proto);
		return;
	}
	

	if (strcasecmp(s, "upload") == 0)
	{
		log->log("sending [%s]...\n", s);
		log->incIndent();

		rhea::app::FileTransfer::Handle handle;
		if (ftransf->startFileUpload(ch, proto, rhea::getTimeNowMSec(), "C:/rhea/rheaSRC/gpu-fts-nestle-2019/bin/test_upload_big_file.gif", "test", &handle))
			log->log("file transfer started. Handle [0x%08X]\n", handle.asU32());
		else
			log->log("file transfer FAILED to start\n");
		log->decIndent();
		return;
	}

	log->outText(true,false,false,"unknown command [%s]\n", s);

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

/*****************************************************
 * true se il main deve aspettare per un char prima di terminare
 * false se il main termina all'istante
 */
bool run (const sThreadInitParam *init)
{
	HThreadMsgR msgQHandleR = init->handleR;
	WinTerminal *log = init->wt;

	const char			IP[] = { "127.0.0.1" };
	const int			PORT_NUMBER = 2280;
	rhea::Allocator		*localAllocator = rhea::memory_getDefaultAllocator();
	
	//apro una socket
	OSSocket sok;
	log->log("opening socket %s:%d...", IP, PORT_NUMBER);
	eSocketError err = OSSocket_openAsTCPClient(&sok, IP, PORT_NUMBER);
	if (eSocketError_none != err)
	{
		log->log("FAILED with errCode=%d\n", (u32)err);
		return true;
	}

	//canale di comunicazione
	rhea::ProtocolChSocketTCP ch(localAllocator, 4096, sok);

	//Protocollo 
	rhea::ProtocolConsole proto(localAllocator, 1024);

	//handshake
	log->log("sending handshake to SMU\n");
	log->incIndent();
	if (!proto.handshake_clientSend(&ch, log))
	{
		log->log("FAIL\n");
		log->decIndent();
		return true;
	}

	//siamo connessi
	log->log ("socket connected!\n");
	log->decIndent();


	//Setup della waitableGrpo
	OSWaitableGrp waitGrp;
	waitGrp.addSocket(sok, u32MAX);

	{
		OSEvent hMsgQEvent;
		rhea::thread::getMsgQEvent (msgQHandleR, &hMsgQEvent);
		waitGrp.addEvent(hMsgQEvent, u32MAX);
	}


	//creazione del buffer di ricezione
	rhea::LinearBuffer	bufferR;
	bufferR.setup(localAllocator, 8192);


	if(identify(&ch, &proto, bufferR, log))
	{
		//handler dei file transfer
		rhea::app::FileTransfer fileTransferManager;
		fileTransferManager.setup(localAllocator);

		bool bQuit = false;
		while (bQuit == false)
		{
			//ogni 10 secondi mi sblocco indipendentemente dall'avere ricevuto notifiche o meno
			u8 nEvent = waitGrp.wait (10000);

			//vediamo cosa mi ha svegliato
			for (u8 i = 0; i < nEvent; i++)
			{
				if (OSWaitableGrp::evt_origin_socket == waitGrp.getEventOrigin(i))
				{
					//ho qualcosa sulla socket
					handleMsgFromSocketServer(&ch, &proto, bufferR, &fileTransferManager, log);
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
							handleUserInput((const char*)msg.buffer, &ch, &proto, log, &fileTransferManager);
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
					log->log("FTE => handle[0x%08X] status[%d=%s] totalBytes[%d] currentBytes[%d] failReason[%d=%s]\n", 
									handle, 
									info.status, rhea::app::utils::verbose_fileTransferStatus(info.status),
									info.totalTransferSizeInBytes, 
									info.currentTransferSizeInBytes,
									info.failReason, rhea::app::utils::verbose_fileTransferFailReason(info.failReason)
							);
				}
			}

		}

		fileTransferManager.unsetup();
	}
	bufferR.unsetup();


	//chiudo
	log->log("closing connection\n");
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


