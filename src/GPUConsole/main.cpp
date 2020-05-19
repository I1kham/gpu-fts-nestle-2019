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
#define BROADCAST_PORTNUMBER		2281

socketbridge::SokBridgeClientVer	version;
socketbridge::SokBridgeIDCode		idCode;
UserCommandFactory					userCommandFactory;
u32									SMUVersion = 0;
bool								bQuitMainThread;
sIPAddressAndSubnetMask				*ipList = NULL;
u8									nIPList = 0;

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
 *	se ip non è specificato, assume di default IP=127.0.0.1 e PORT=DEFAULT_PORT_NUMBER
 *	se ip è specificato e port non lo è, assume PORT=DEFAULT_PORT_NUMBER
 */
bool handleCommandSyntax_connect (WinTerminal *logger, const char *s, char *out_ip, u32 sizeofIP, u16 *out_port)
{
	sprintf_s (out_ip, sizeofIP, "127.0.0.1");
	*out_port = DEFAULT_PORT_NUMBER;

	rhea::string::utf8::Iter iter;
	rhea::string::utf8::Iter iter2;
	
	iter.setup((const u8*)s);

	const rhea::UTF8Char cSpace(" ");
	rhea::string::utf8::advanceUntil(iter, &cSpace, 1);
	rhea::string::utf8::toNextValidChar(iter);

	//se c'è un ip
	if (rhea::string::utf8::extractValue(iter, &iter2, &cSpace, 1))
	{
		iter2.copyAllStr((u8*)out_ip, sizeofIP);

		//ci devono essere 3 . e il resto solo numeri
		u8 nPunti = 0;
		u8 n = (u8)strlen(out_ip);
		for (u8 i = 0; i < n; i++)
		{
			if (out_ip[i] == '.')
			{
				nPunti++;
				continue;
			}
			if (out_ip[i]<'0' || out_ip[i]>'9')
			{
				logger->log("invalid parameter. Valid parameter is ip-addres blank port-number. Port-number is optional\n");
				return false;
			}
		}
		if (nPunti != 3)
		{
			logger->log("invalid parameter. Valid parameter is ip-addres blank port-number. Port-number is optional\n");
			return false;
		}


		rhea::string::utf8::toNextValidChar(iter);

		i32 port;
		if (rhea::string::utf8::extractInteger(iter, &port))
		{
			if (port > 0 && port < 65536)
				*out_port = (u16)port;
		}
		return true;
	}


	return true;

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
void handleCommandHello (WinTerminal *logger)
{
	OSSocket sokUDP;
	rhea::socket::init(&sokUDP);
	rhea::socket::openAsUDP(&sokUDP);

	u8 buffer[32];
	u8 ct = 0;
	buffer[ct++] = 'R';
	buffer[ct++] = 'H';
	buffer[ct++] = 'E';
	buffer[ct++] = 'A';
	buffer[ct++] = 'H';
	buffer[ct++] = 'E';
	buffer[ct++] = 'l';
	buffer[ct++] = 'l';
	buffer[ct++] = 'O';

	logger->incIndent();

	for (u8 i = 0; i < nIPList; i++)
	{
		if (strcmp(ipList[i].ip, "127.0.0.1") == 0)
			continue;

		logger->log("broadcasting [%s] with subnet mask [%s] ", ipList[i].ip, ipList[i].subnetMask);
		rhea::socket::UDPSendBroadcast(sokUDP, buffer, ct, ipList[i].ip, BROADCAST_PORTNUMBER, ipList[i].subnetMask);
		
		u64 timeToExitMSec = rhea::getTimeNowMSec() + 3000;
		while (rhea::getTimeNowMSec() < timeToExitMSec)
		{
			OSNetAddr from;
			u8 buffer[32];

			logger->log(".");
			rhea::thread::sleepMSec(100);

			u32 nBytesRead = rhea::socket::UDPReceiveFrom(sokUDP, buffer, sizeof(buffer), &from);
			if (nBytesRead >= 9)
			{
				if (memcmp(buffer, "rheaHelLO", 9) == 0)
				{
					char ip[32];

					rhea::netaddr::getIPv4(from, ip);
					if (strcmp(ipList[i].ip, ip) != 0)
					{
						//int port = rhea::netaddr::getPort(from);
						logger->incIndent();
						logger->log("\nrcv answer from [%s] ", ip);
						logger->decIndent();
						timeToExitMSec = rhea::getTimeNowMSec() + 3000;
					}
				}
			}
		}
		logger->log("\n");
	}
	logger->decIndent();
	logger->log("Finished\n");
	rhea::socket::close(sokUDP);
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

	case socketbridge::eEventType_creditUpdated:
		{
			char credit[16];
			rhea::app::CurrentCredit::decodeAnswer(decoded, (u8*)&credit, sizeof(credit));
			log->outText(true, true, false, "RCV [creditChange] => [%s]\n", credit);
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
			u16 flag1 = 0;

			rhea::app::CurrentCPUStatus::decodeAnswer (decoded, &vmcState, &vmcErrorCode, &vmcErrorType, &flag1);
			log->outText (true, true, false, "RCV [cpuStatus] => state=[%d %s], err_code=[%d], err_type=[%d], flag[%04X]\n", vmcState, rhea::app::utils::verbose_eVMCState(vmcState), vmcErrorCode, vmcErrorType, flag1);
		}
		break;

	case socketbridge::eEventType_reqClientList:
		{
			log->outText(true, true, false, "RCV [reqClientList]\n");
			log->incIndent();
			rhea::Allocator *allocator = rhea::getScrapAllocator();

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

	case socketbridge::eEventType_btnProgPressed:
		rhea::app::ButtonProgPressed::decodeAnswer(decoded);
		log->outText(true, true, false, "RCV [btn prog pressed]\n");
		break;

	case socketbridge::eEventType_reqDataAudit:
		{
			cpubridge::eReadDataFileStatus status;
			u16 toKbSoFar;
			u16 fileID;
			rhea::app::ReadDataAudit::decodeAnswer(decoded, &status, &toKbSoFar, &fileID);

			log->outText(true, true, false, "readDataAudit: status[%s] totKbSoFar[%d] fileID[%d]\n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);
		}
		break;

	case socketbridge::eEventType_reqIniParam:
		{
			cpubridge::sCPUParamIniziali iniParam;
			rhea::app::CurrentCPUInitParam::decodeAnswer(decoded, &iniParam);
			log->outText(true, true, false, "RCV [iniParam]: CPU_ver[%s] protocol_ver[%d]\n", iniParam.CPU_version, iniParam.protocol_version);
		}
		break;

	case socketbridge::eEventType_reqVMCDataFile:
		{
			cpubridge::eReadDataFileStatus status;
			u16 toKbSoFar;
			u16 fileID;
			rhea::app::ReadVMCDataFile::decodeAnswer(decoded, &status, &toKbSoFar, &fileID);
			log->outText(true, true, false, "readVMCDataFile: status[%s] totKbSoFar[%d] fileID[%d]\n", rhea::app::utils::verbose_readDataFileStatus(status), toKbSoFar, fileID);
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
			log->outText(true, true, false, text);
		}
		break;

	case socketbridge::eEventType_reqWriteLocalVMCDataFile:
		{
			cpubridge::eWriteDataFileStatus status;
			u16 toKbSoFar;
			rhea::app::WriteLocalVMCDataFile::decodeAnswer(decoded, &status, &toKbSoFar);
			log->outText(true, true, false, "WriteLocalVMCDataFile: status[%s] totKbSoFar[%d]\n", rhea::app::utils::verbose_writeDataFileStatus(status), toKbSoFar);
		}
		break;

	case socketbridge::eEventType_cpuSanWashingStatus:
		{
			u8 b0, b1, b2;
			rhea::app::SanWashingStatus::decodeAnswer (decoded, &b0, &b1, &b2);
			log->outText(true, true, false, "SanWashingStatus: fase[%d] btn1[%d] btn2[%d]\n", b0, b1, b2);
		}
		break;

	case socketbridge::eEventType_cpuWritePartialVMCDataFile:
		{
			u8 blockWritten = 0;
			rhea::app::WritePartialVMCDataFile::decodeAnswer(decoded, &blockWritten);
			log->outText(true, true, false, "PArtial DA3 written: block[%d]\n", blockWritten);
		}
		break;

	case socketbridge::eEventType_cpuExtendedConfigInfo:
		{
			cpubridge::sExtendedCPUInfo info;			
			rhea::app::ExtendedConfigInfo::decodeAnswer(decoded, &info);
			log->outText(true, true, false, "cpuExtendedConfigInfo: ver[%d] type[%s] model[%d] induzione?[%d]\n", info.msgVersion, rhea::app::utils::verbose_CPUMachineType(info.machineType), info.machineModel, info.isInduzione);
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
			strcat_s(s, sizeof(s), "\n");
			log->outText(true, true, false, s);
		}
		break;		

	case socketbridge::eEventType_setDecounter:
		{
			cpubridge::eCPUProgrammingCommand_decounter which;
			u16 value = 0;
			rhea::app::SetDecounter::decodeAnswer(decoded, &which, &value);
			log->outText(true, true, false, "setDecounter: which[%d] value[%d]\n", (u8)which, value);
		}
		break;		

	case socketbridge::eEventType_getAperturaVGrind:
		{
			u8 macina_1o2 = 0;
			u16 value = 0;
			rhea::app::GetAperturaVGrind::decodeAnswer(decoded, &macina_1o2, &value);
			log->outText(true, true, false, "GetAperturaVGrind: macina[%d] value[%d]\n", macina_1o2, value);
		}
		break;

	case socketbridge::eEventType_setMotoreMacina:
		{
			u8 macina_1o2 = 0;
			cpubridge::eCPUProgrammingCommand_macinaMove m;
			rhea::app::SetMotoreMacina::decodeAnswer(decoded, &macina_1o2, &m);
			log->outText(true, true, false, "SetMotoreMacina: macina[%d] movimento[%d]\n", macina_1o2, (u8)m);
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

	if (strncmp(s, "hello", 5) == 0)
	{
		handleCommandHello(log);
		return 0;
	}

	if (!ch->isOpen())
	{
		log->outText(true, false, false, "not connected\n");
		return 0;
	}

	if (strcasecmp(s, "disconnect") == 0)
		return 0x02;



	if (strcasecmp(s, "upload-da3-1") == 0)
	{
		//shortcut di comodo per uppare uno specifico file da3 che uso come test
		char s[256];
		sprintf_s(s, sizeof(s), "upload C:\\Users\\gbrunelli\\AppData\\Roaming\\rheaConsole\\lucrezia_001.da3");
		log->log("%s\n", s);
		userCommandFactory.handle(s, ch, proto, log, ftransf);
		return 0;
	}
	if (strcasecmp(s, "upload-da3-2") == 0)
	{
		//shortcut di comodo per uppare uno specifico file da3 che uso come test
		char s[256];
		sprintf_s(s, sizeof(s), "upload C:\\Users\\gbrunelli\\AppData\\Roaming\\rheaConsole\\lucrezia_002.da3");
		log->log("%s\n", s);
		userCommandFactory.handle(s, ch, proto, log, ftransf);
		return 0;
	}

	if (strcasecmp(s, "install-da3-1") == 0)
	{
		//shortcut di comodo per chiedere alla SMU di installare il da3 uppato con "upload-da3-1"
		char s[64];
		sprintf_s(s, sizeof(s), "APP:/temp/lucrezia_001.da3");
		log->log("install %s\n", s);
		rhea::app::WriteLocalVMCDataFile::ask(ch, proto, s);
		return 0;
	}
	if (strcasecmp(s, "install-da3-2") == 0)
	{
		//shortcut di comodo per chiedere alla SMU di installare il da3 uppato con "upload-da3-1"
		char s[64];
		sprintf_s(s, sizeof(s), "APP:/temp/lucrezia_002.da3");
		log->log("install %s\n", s);
		rhea::app::WriteLocalVMCDataFile::ask(ch, proto, s);
		return 0;
	}

	if (userCommandFactory.handle(s, ch, proto, log, ftransf))
		return 0;

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
	eSocketError err = rhea::socket::openAsTCPClient (&sok, IP, PORT_NUMBER);
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
	rhea::Allocator	*localAllocator = rhea::getScrapAllocator();

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
								if (handleCommandSyntax_connect(logger, (const char*)msg.buffer, ip, sizeof(ip), &port))
								{
									if (connect(&ch, &proto, bufferR, ip, port, logger))
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
#include "Iphlpapi.h"
#include "ws2tcpip.h"
#pragma comment(lib, "IPHLPAPI.lib")
sIPAddressAndSubnetMask* scanNetworkAdaptersAndFindLocalIP (rhea::Allocator *allocator, u8 *out_nRecordFound)
{
	*out_nRecordFound = 0;

	rhea::Allocator *tempAllocator = rhea::getScrapAllocator();

	// Before calling AddIPAddress we use GetIpAddrTable to get an adapter to which we can add the IP.
	DWORD dwTableSize = sizeof(MIB_IPADDRTABLE);
	PMIB_IPADDRTABLE pIPAddrTable = (MIB_IPADDRTABLE *)RHEAALLOC(tempAllocator,dwTableSize);
	if (GetIpAddrTable(pIPAddrTable, &dwTableSize, 0) == ERROR_INSUFFICIENT_BUFFER)
	{
		RHEAFREE(tempAllocator, pIPAddrTable);
		pIPAddrTable = (MIB_IPADDRTABLE *)RHEAALLOC(tempAllocator, dwTableSize);
	}

	// Make a second call to GetIpAddrTable to get the actual data we want
	DWORD err = GetIpAddrTable(pIPAddrTable, &dwTableSize, 0);
	if (err != NO_ERROR) 
	{
		/*
		LPVOID lpMsgBuf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)& lpMsgBuf, 0, NULL))
		{
			printf("\tError: %s", (const char*)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
		*/
		return NULL;
	}

	if (pIPAddrTable->dwNumEntries == 0)
		return NULL;
	if (pIPAddrTable->dwNumEntries > 250)
		*out_nRecordFound = 250;
	else
		*out_nRecordFound = (u8)pIPAddrTable->dwNumEntries;
	
	sIPAddressAndSubnetMask *ret = (sIPAddressAndSubnetMask*)RHEAALLOC(allocator, sizeof(sIPAddressAndSubnetMask) * (*out_nRecordFound));
	memset(ret, 0, sizeof(sIPAddressAndSubnetMask) * (*out_nRecordFound));
	for (u8 i = 0; i < (*out_nRecordFound); i++)
	{
		IN_ADDR IPAddr;

		//printf("\n\tInterface Index[%d]:\t%ld\n", i, pIPAddrTable->table[i].dwIndex);
		
		IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwAddr;
		InetNtopA(AF_INET, &IPAddr, ret[i].ip, sizeof(ret[i].ip));
		printf("\tIP Address[%d]:     \t%s\n", i, ret[i].ip);
		
		IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwMask;
		InetNtopA(AF_INET, &IPAddr, ret[i].subnetMask, sizeof(ret[i].subnetMask));
		printf("\tSubnet Mask[%d]:    \t%s\n", i, ret[i].subnetMask);
		
		/*IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwBCastAddr;
		printf("\tBroadCast[%d]:      \t%s (%ld)\n", i, InetNtop(AF_INET, &IPAddr, sAddr, sizeof(sAddr)), pIPAddrTable->table[i].dwBCastAddr);
		
		printf("\tReassembly size[%d]:\t%ld\n", i, pIPAddrTable->table[i].dwReasmSize);
		
		printf("\tType and State[%d]:", i);
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_PRIMARY)
			printf("\tPrimary IP Address");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DYNAMIC)
			printf("\tDynamic IP Address");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DISCONNECTED)
			printf("\tAddress is on disconnected interface");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_DELETED)
			printf("\tAddress is being deleted");
		
		if (pIPAddrTable->table[i].wType & MIB_IPADDR_TRANSIENT)
			printf("\tTransient address");*/
		//printf("\n");
	}

	if (pIPAddrTable) 
		RHEAFREE(tempAllocator, pIPAddrTable);

	return ret;
}

//*****************************************************
int main()
{
	HINSTANCE hInst = NULL;
	rhea::init("rheaConsole", &hInst);

	//elenco delle schede di rete e relativi ip/subnet mask. Serve per il broadcast su tutte le reti del comando hello
	rhea::Allocator *localAllocator = rhea::getScrapAllocator();
	ipList = scanNetworkAdaptersAndFindLocalIP(localAllocator, &nIPList);

	//version info
	version.apiVersion = 0x01;
	version.appType = socketbridge::SokBridgeClientVer::APP_TYPE_CONSOLE;
	version.unused2 = 0x00;
	version.unused3 = 0x00;

	//idCode della connessione a SocketBridge
	memset(&idCode, 0, sizeof(idCode));

	go();
	
	if (ipList)
		RHEAFREE(localAllocator, ipList);
	
	rhea::deinit();
	return 0;
}


