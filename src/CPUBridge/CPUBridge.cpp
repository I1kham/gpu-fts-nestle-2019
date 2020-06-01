#include "CPUBridge.h"
#include "CPUBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaNetBufferView.h"

struct sThreadInitParam
{
	rhea::ISimpleLogger *logger;
	cpubridge::CPUChannel *chToCPU;
	HThreadMsgR chToThreadR;
	HThreadMsgW chToThreadW;
	OSEvent	hEvThreadStarted;
};


i16     cpuCommThreadFn (void *userParam);


//****************************************************************************
bool cpubridge_helper_folder_create (const char *folder, rhea::ISimpleLogger *logger)
{
    char s[512];
    sprintf_s(s, sizeof(s), "%s/%s", rhea::getPhysicalPathToAppFolder(), folder);
    if (!rhea::fs::folderCreate((const u8*)s))
    {
        logger->log ("ERR: can't create folder [%s]\n", s);
        return false;
    }
    else
    {
        logger->log ("CREATED folder [%s]\n", s);
        return true;
    }
}

//****************************************************************************
bool cpubridge::startServer (CPUChannel *chToCPU, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread, HThreadMsgW *out_hServiceChannelW)
{
	//creo la struttura di cartelle necessarie al corretto funzionamento
    cpubridge_helper_folder_create("current", logger);
    cpubridge_helper_folder_create("current/lang", logger);
    cpubridge_helper_folder_create("current/da3", logger);
    cpubridge_helper_folder_create("current/cpu", logger);
    cpubridge_helper_folder_create("last_installed", logger);
    cpubridge_helper_folder_create("/last_installed/da3", logger);
    cpubridge_helper_folder_create("last_installed/cpu", logger);
    cpubridge_helper_folder_create("temp", logger);

    char s[512];
    sprintf_s(s, sizeof(s), "%s/temp", rhea::getPhysicalPathToAppFolder());
	rhea::fs::deleteAllFileInFolderRecursively((const u8*)s, false);

	
	
	sThreadInitParam    init;

	//creo una coda FIFO da associare al thread in modo che sia possibile comunicare con il thread stesso
	rhea::thread::createMsgQ (&init.chToThreadR, &init.chToThreadW);


	//crea il thread
	init.logger = logger;
	init.chToCPU = chToCPU;
	rhea::event::open (&init.hEvThreadStarted);
	rhea::thread::create (out_hThread, cpuCommThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = rhea::event::wait (init.hEvThreadStarted, 1000);
	rhea::event::close(init.hEvThreadStarted);

	if (bStarted)
	{
		*out_hServiceChannelW = init.chToThreadW;
		return true;
	}

	return false;
}

//***************************************************
void cpubridge::loadVMCDataFileTimeStamp (sCPUVMCDataFileTimeStamp *out)
{
    out->setInvalid();

    char s[512];
    sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.timestamp", rhea::getPhysicalPathToAppFolder());
    FILE *f = fopen(s, "rb");
    if (NULL == f)
        return;
    out->readFromFile(f);
    fclose(f);
}

//***************************************************
bool cpubridge::saveVMCDataFileTimeStamp(const sCPUVMCDataFileTimeStamp &ts)
{
    char s[512];
    sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.timestamp", rhea::getPhysicalPathToAppFolder());
    FILE *f = fopen(s, "wb");
    if (NULL == f)
        return false;
    ts.writeToFile(f);
    fclose(f);
    return true;
}

//*****************************************************************
i16 cpuCommThreadFn (void *userParam)
{
	sThreadInitParam *init = (sThreadInitParam*)userParam;
	HThreadMsgR chToThreadR = init->chToThreadR;
	HThreadMsgW chToThreadW = init->chToThreadW;

	cpubridge::Server server;
	server.useLogger(init->logger);
	if (server.start (init->chToCPU, chToThreadR))
	{
		//segnalo che il thread � partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();

	rhea::thread::deleteMsgQ (chToThreadR, chToThreadW);
	return 1;
}


/***************************************************
 * ritorna 0 se out_buffer non � abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 */
u8 cpubridge_buildMsg (cpubridge::eCPUCommand command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	//calcolo della dimensione totale
	if (sizeOfOutBuffer < 4 + sizeOfOptionaData)
		return 0;

	u8 ct = 0;
	out_buffer[ct++] = '#';
	out_buffer[ct++] = (u8)command;
	out_buffer[ct++] = 0;	//length

	if (optionalData && sizeOfOptionaData)
	{
		memcpy(&out_buffer[ct], optionalData, sizeOfOptionaData);
		ct += sizeOfOptionaData;
	}

	out_buffer[2] = (ct+1);	//length
	out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
	ct++;
	
	return ct;
}

//***************************************************
u8 cpubridge::buildMsg_checkStatus_B (u8 keyPressed, u8 langErrorCode, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[8];
	u8 ct = 0;
	optionalData[ct++] = keyPressed;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = langErrorCode;

	return cpubridge_buildMsg (cpubridge::eCPUCommand_checkStatus_B, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startSelectionWithPaymentAlreadyHandledByGPU_V (u8 selNum, u16 prezzo, ePaymentMode paymentMode, eGPUPaymentType paymentType, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[8];
	u8 ct = 0;
	optionalData[ct++] = selNum;
	rhea::utils::bufferWriteU16_LSB_MSB (&optionalData[ct], prezzo);
	ct += 2;
	optionalData[ct++] = (u8)paymentMode;
	optionalData[ct++] = (u8)paymentType;

	return cpubridge_buildMsg (cpubridge::eCPUCommand_startSelWithPaymentAlreadyHandled_V, optionalData, ct, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_setDecounter (eCPUProgrammingCommand_decounter which, u16 valore, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[3];
	optionalData[0] = (u8)which;
	optionalData[1] = (u8)(valore & 0x00FF);			//LSB
	optionalData[2] = (u8)((valore & 0xFF00) >> 8);		//MSB
	
	return buildMsg_Programming(eCPUProgrammingCommand_setDecounter, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getAllDecounterValues(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getAllDecounterValues, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_calcolaImpulsiGruppo (u8 macina_1o2, u16 totalePesata_dGrammi, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];

	if (macina_1o2 == 2)
		optionalData[0] = 12;
	else
		optionalData[0] = 11;
	rhea::utils::bufferWriteU16_LSB_MSB(&optionalData[1], totalePesata_dGrammi);
	return buildMsg_Programming(eCPUProgrammingCommand_calcolaImpulsiMacina, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoCalcoloImpulsiGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getStatoCalcoloImpulsi, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getTime(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getTime, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getDate(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getDate, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setTime(u8 *out_buffer, u8 sizeOfOutBuffer, u8 hh, u8 mm, u8 ss)
{
	u8 optionalData[4];
	optionalData[0] = hh;
	optionalData[1] = mm;
	optionalData[2] = ss;
	return buildMsg_Programming(eCPUProgrammingCommand_setTime, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setDate(u8 *out_buffer, u8 sizeOfOutBuffer, u16 year, u8 month, u8 day)
{
	u8 optionalData[4];
	optionalData[0] = (u8)(year-2000);
	optionalData[1] = month;
	optionalData[2] = day;
	return buildMsg_Programming(eCPUProgrammingCommand_setDate, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getPosizioneMacina(u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1o2)
{
	if (macina_1o2 != 2)
		macina_1o2 = 1;

	if (macina_1o2 == 1)
		macina_1o2 = 11;
	else
		macina_1o2 = 12;

	u8 optionalData[2];
	optionalData[0] = macina_1o2;
	return buildMsg_Programming(eCPUProgrammingCommand_getPosizioneMacina, optionalData, 1, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setMotoreMacina(u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m)
{
	if (macina_1o2 != 2)
		macina_1o2 = 1;

	if (macina_1o2 == 1)
		macina_1o2 = 11;
	else
		macina_1o2 = 12;

	u8 optionalData[2];
	optionalData[0] = macina_1o2;
	optionalData[1] = (u8)m;
	return buildMsg_Programming(eCPUProgrammingCommand_setMotoreMacina, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_testSelection (u8 *out_buffer, u8 sizeOfOutBuffer, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d)
{
	u8 optionalData[4];
	optionalData[0] = selNum;
	optionalData[1] = (u8)d;

	return buildMsg_Programming(eCPUProgrammingCommand_testSelezione, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getNomiLingueCPU(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getNomiLinguaCPU, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_disintallazione(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_disinstallazione, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_ricaricaFasciaOrariaFreevend(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_ricaricaFasciaOrariaFV, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_EVAresetPartial(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_EVAresetPartial, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_EVAresetTotals(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_EVAresetTotals, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_getVoltAndTemp(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getVoltAndTemp, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getCPUOFFReportDetails(u8 *out_buffer, u8 sizeOfOutBuffer, u8 indexNum)
{
	u8 optionalData[4];
	optionalData[0] = indexNum;
	return buildMsg_Programming(eCPUProgrammingCommand_getCPUOFFReportDetails, optionalData, 1, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getLastFluxInformation(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getLastFluxInformation, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getCPUStringVersionAndModel(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getStringVersionAndModel, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startModemTest(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_startModemTest, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_startTestAssorbGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_startTestAssorbMotoriduttore, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_attivazioneMotore(u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	optionalData[0] = motore_1_10;
	optionalData[1] = durata_dSec;
	optionalData[2] = numRipetizioni;
	optionalData[3] = pausaTraRipetizioni_dSec;

	return buildMsg_Programming(eCPUProgrammingCommand_attivazioneMotore, optionalData, 4, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setFattoreCalibMotore(eCPUProgrammingCommand_motor motore, u16 valoreInGr, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	optionalData[0] = (u8)motore;
	rhea::utils::bufferWriteU16_LSB_MSB(&optionalData[1], valoreInGr);
	return buildMsg_Programming(eCPUProgrammingCommand_setFattoreCalibrazioneMotore, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getExtendedConfigInfo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand_getExtendedConfigInfo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getStatoGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getStatusTestAssorbGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getStatusTestAssorbMotoriduttore, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_initialParam_C(u8 gpuVersionMajor, u8 gpuVersionMinor, u8 gpuVersionBuild, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	u8 ct = 0;

	optionalData[ct++] = gpuVersionMajor;
	optionalData[ct++] = gpuVersionMinor;
	optionalData[ct++] = gpuVersionBuild;

	return cpubridge_buildMsg (cpubridge::eCPUCommand_initialParam_C, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_restart_U(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg (cpubridge::eCPUCommand_restart, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_readDataAudit (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    u8 optionalData[2] = {0,0};
    return cpubridge_buildMsg (cpubridge::eCPUCommand_readDataAudit, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_readVMCDataFile(u8 blockNum, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[2] = { blockNum, 0 };
	return cpubridge_buildMsg(cpubridge::eCPUCommand_readVMCDataFile, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_writeVMCDataFile(const u8 *buffer64yteLettiDalFile, u8 blockNum, u8 totNumBlocks, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[VMCDATAFILE_BLOCK_SIZE_IN_BYTE + 2];
	optionalData[0] = blockNum;
	optionalData[1] = totNumBlocks;
	memcpy(&optionalData[2], buffer64yteLettiDalFile, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	return cpubridge_buildMsg(cpubridge::eCPUCommand_writeVMCDataFile, optionalData, sizeof(optionalData), out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_writePartialVMCDataFile(const u8 *buffer64byte, u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[VMCDATAFILE_BLOCK_SIZE_IN_BYTE + 3];
	optionalData[0] = blocco_n_di;
	optionalData[1] = tot_num_blocchi;
	optionalData[2] = blockNumOffset;
	memcpy(&optionalData[3], buffer64byte, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	return cpubridge_buildMsg(cpubridge::eCPUCommand_writePartialVMCDataFile, optionalData, sizeof(optionalData), out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getVMCDataFileTimeStamp (u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand_getVMCDataFileTimeStamp, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getMilkerVer(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand_getMilkerVer, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_Programming (eCPUProgrammingCommand cmd, const u8 *optionalDataIN, u32 sizeOfOptionalDataIN, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	assert(sizeOfOptionalDataIN < 31);
    u8 optionalData[32];
    optionalData[0] = (u8)cmd;
	if (NULL != optionalDataIN && sizeOfOptionalDataIN > 0)
		memcpy(&optionalData[1], optionalDataIN, sizeOfOptionalDataIN);
    return cpubridge_buildMsg (cpubridge::eCPUCommand_programming, optionalData, 1+ sizeOfOptionalDataIN, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getTimeNextLavaggioSanCappuccinatore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand_getTimeNextLavaggioCappuccinatore, NULL, 0, out_buffer, sizeOfOutBuffer);
}





//***************************************************
void cpubridge::subscribe(const HThreadMsgW &hCPUMsgQWrite, const HThreadMsgW &hOtherMsgQWrite)
{
	u32 param32 = hOtherMsgQWrite.asU32();
	rhea::thread::pushMsg (hCPUMsgQWrite, CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST, param32);
}

//***************************************************
void cpubridge::translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, sSubscriber *out, u8 *out_cpuBridgeVersion)
{
	assert(msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER);
	memcpy(out, msg.buffer, sizeof(sSubscriber));
	*out_cpuBridgeVersion = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::unsubscribe(const sSubscriber &sub)
{
	rhea::thread::pushMsg(sub.hFromCpuToOtherW, CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST, (u32)0);
}


//***************************************************
void cpubridge::notify_CPUBRIDGE_DYING (const sSubscriber &to)
{
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_DYING, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_STATE_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, cpubridge::eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType, u16 flag1)
{
	logger->log("notify_CPU_STATE_CHANGED\n");

	u8 state[8];
	state[0] = (u8)VMCstate;
	state[1] = (u8)VMCerrorCode;
	state[2] = (u8)VMCerrorType;
	rhea::utils::bufferWriteU16(&state[3], flag1);
	rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED, handlerID, state, 5);
}

//***************************************************
void cpubridge::translateNotify_CPU_STATE_CHANGED(const rhea::thread::sMsg &msg, cpubridge::eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType, u16 *out_flag1)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED);
	
	const u8 *state = (u8*)msg.buffer;
	*out_VMCstate = (cpubridge::eVMCState)state[0];
	*out_VMCerrorCode = state[1];
	*out_VMCerrorType = state[2];
	*out_flag1 = rhea::utils::bufferReadU16(&state[3]);
}

//***************************************************
void cpubridge::notify_CPU_CREDIT_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const void *credit, u8 sizeOfCredit)
{
	logger->log("notify_CPU_CREDIT_CHANGED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED, handlerID, credit, sizeOfCredit);
}

//***************************************************
void cpubridge::translateNotify_CPU_CREDIT_CHANGED(const rhea::thread::sMsg &msg, u8 *out_credit, u16 sizeOfOut)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED);
	u32 len = msg.bufferSize;
	if (sizeOfOut <= len)
		len = sizeOfOut;
	memcpy(out_credit, msg.buffer, len);
}

//***************************************************
void cpubridge::notify_CPU_NEW_LCD_MESSAGE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPULCDMessage *msg)
{
	logger->log("notify_CPU_NEW_LCD_MESSAGE\n");

	for (u32 i = 0; i < sCPULCDMessage::BUFFER_SIZE_IN_U16; i++)
	{
		if (msg->utf16LCDString[i] == 0)
		{
			//rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, handlerID, msg, sizeof(sCPULCDMessage));
			rhea::thread::pushMsg2Buffer(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, handlerID, &msg->importanceLevel, 1, msg, i*2);
			return;
		}
	}
}

//***************************************************
void cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE (const rhea::thread::sMsg &msg, sCPULCDMessage *out_msg)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE);
	//memcpy(out_msg, msg.buffer, msg.bufferSize);

	const u8 *p = (const u8*)msg.buffer;
	out_msg->importanceLevel = p[0];
	
	memset (out_msg->utf16LCDString, 0, sizeof(out_msg->utf16LCDString));
	if (msg.bufferSize > 1)
		memcpy(out_msg->utf16LCDString, &p[1], msg.bufferSize-1);
}

//***************************************************
void cpubridge::notify_CPU_SEL_AVAIL_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUSelAvailability *selAvailability)
{
	logger->log("notify_CPU_SEL_AVAIL_CHANGED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED, handlerID, selAvailability->_flag, sizeof(selAvailability->_flag));
}

//***************************************************
void cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(const rhea::thread::sMsg &msg, sCPUSelAvailability *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED);
	memcpy(out->_flag, msg.buffer, sizeof(out->_flag));
}


//***************************************************
void cpubridge::notify_CPU_SEL_PRICES_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 numPrices, u8 numDecimals, const u16 *prices)
{
	logger->log("notify_CPU_SEL_PRICES_CHANGED\n");
	u16 buffer[NUM_MAX_SELECTIONS + 1];
	if (numPrices > NUM_MAX_SELECTIONS)
		numPrices = NUM_MAX_SELECTIONS;
	
	buffer[0] = (u16)numPrices | ( (u16)numDecimals << 8);
	memcpy(&buffer[1], prices, sizeof(u16)* numPrices);

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED, handlerID, buffer, sizeof(u16) * (numPrices+1) );
}

//***************************************************
void cpubridge::translateNotify_CPU_SEL_PRICES_CHANGED(const rhea::thread::sMsg &msg, u8 *out_numPrices, u8 *out_numDecimals, u16 *out_prices)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED);
	const u16 *p = (const u16*)msg.buffer;
	*out_numPrices = (u8)(p[0] & 0x00FF);
	*out_numDecimals = (u8)((p[0] & 0xFF00)>>8);
	memcpy(out_prices, &p[1], sizeof(u16)* (*out_numPrices));
}

//***************************************************
void cpubridge::notify_CPU_RUNNING_SEL_STATUS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eRunningSelStatus status)
{
	logger->log("notify_CPU_RUNNING_SEL_STATUS [%d]\n", (u8)status);

	u32 paramU32 = status;
	paramU32 <<= 16;
	paramU32 |= handlerID;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS, paramU32);
}

//***************************************************
void cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg, eRunningSelStatus *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS);
	*out_s = (eRunningSelStatus)((msg.paramU32 & 0xFFFF0000) >> 16);
}

//***************************************************
void cpubridge::notify_CPU_FULLSTATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUStatus *s)
{
	logger->log("notify_CPU_FULLSTATE\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_FULLSTATE, handlerID, s, sizeof(sCPUStatus));
}

//***************************************************
void cpubridge::translateNotify_CPU_FULLSTATE(const rhea::thread::sMsg &msg, sCPUStatus *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_FULLSTATE);
	memcpy(out_s, msg.buffer, sizeof(sCPUStatus));
}

//***************************************************
void cpubridge::notify_CPU_INI_PARAM(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUParamIniziali *s)
{
	logger->log("notify_CPU_INI_PARAM\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_INI_PARAM, handlerID, s, sizeof(sCPUParamIniziali));
}

//***************************************************
void cpubridge::translateNotify_CPU_INI_PARAM(const rhea::thread::sMsg &msg, sCPUParamIniziali *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_INI_PARAM);
	memcpy(out_s, msg.buffer, sizeof(sCPUParamIniziali));
}

//***************************************************
void cpubridge::notify_CPU_BTN_PROG_PRESSED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_BTN_PROG_PRESSED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED, handlerID);
}

//***************************************************
void cpubridge::translateNotify_CPU_BTN_PROG_PRESSED(const rhea::thread::sMsg &msg)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED);
}

//***************************************************
void cpubridge::notify_READ_DATA_AUDIT_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID)
{
    logger->log("notify_READ_DATA_AUDIT_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(fileID);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_READ_DATA_AUDIT_PROGRESS (const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID)
{
    assert(msg.what == CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_fileID = u;
	nbr.readU16(u); *out_totKbSoFar = u;
	
    u8 b = 0;
	nbr.readU8(b); *out_status = (eReadDataFileStatus)b;
}

//***************************************************
void cpubridge::notify_READ_VMCDATAFILE_PROGRESS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID)
{
	logger->log("notify_READ_VMCDATAFILE_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(fileID);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_READ_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_fileID = u;
	nbr.readU16(u); *out_totKbSoFar = u;

    u8 b = 0;
	nbr.readU8(b); *out_status = (eReadDataFileStatus)b;
}

//***************************************************
void cpubridge::notify_WRITE_VMCDATAFILE_PROGRESS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eWriteDataFileStatus status, u16 totKbSoFar)
{
	logger->log("notify_WRITE_VMCDATAFILE_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eWriteDataFileStatus *out_status, u16 *out_totKbSoFar)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_totKbSoFar = u;

    u8 b = 0;
	nbr.readU8(b); *out_status = (eWriteDataFileStatus)b;
}

//***************************************************
void cpubridge::notify_WRITE_CPUFW_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, enum eWriteCPUFWFileStatus status, u16 param)
{
	logger->log("notify_WRITE_CPUFW_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eBigEndian);
	nbw.writeU16(param);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_WRITE_CPUFW_PROGRESS(const rhea::thread::sMsg &msg, eWriteCPUFWFileStatus *out_status, u16 *out_param)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_param = u;

    u8 b = 0;
	nbr.readU8(b); *out_status = (eWriteCPUFWFileStatus)b;
}


//***************************************************
void cpubridge::notify_CPU_VMCDATAFILE_TIMESTAMP(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUVMCDataFileTimeStamp &ts)
{
	logger->log("notify_CPU_VMCDATAFILE_TIMESTAMP\n");

	const u8 BUFFER_SIZE = 16;
	u8 buffer[BUFFER_SIZE];
	assert(ts.getLenInBytes() <= BUFFER_SIZE);
	ts.writeToBuffer(buffer);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP, handlerID, buffer, ts.getLenInBytes());
}

//***************************************************
void cpubridge::translateNotify_CPU_VMCDATAFILE_TIMESTAMP(const rhea::thread::sMsg &msg, sCPUVMCDataFileTimeStamp *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP);

	out->readFromBuffer(msg.buffer);
}


//***************************************************
void cpubridge::notify_SAN_WASHING_STATUS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 b0, u8 b1, u8 b2)
{
	logger->log("notify_SAN_WASHING_STATUS\n");
	u8 buffer[4] = { b0, b1, b2, 0 };
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_SAN_WASHING_STATUS(const rhea::thread::sMsg &msg, u8 *out_b0, u8 *out_b1, u8 *out_b2)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS);

	const u8 *p = (const u8*)msg.buffer;
	*out_b0 = p[0];
	*out_b1 = p[1];
	*out_b2 = p[2];
}


//***************************************************
void cpubridge::notify_WRITE_PARTIAL_VMCDATAFILE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 blockNumOffset)
{
	logger->log("notify_WRITE_PARTIAL_VMCDATAFILE [%d]\n", blockNumOffset);
	u8 buffer[4] = { blockNumOffset, 0, 0, 0 };
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_WRITE_PARTIAL_VMCDATAFILE_PROGRESS, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_WRITE_PARTIAL_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_blockNumOffset)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_PARTIAL_VMCDATAFILE_PROGRESS);

	const u8 *p = (const u8*)msg.buffer;
	*out_blockNumOffset = p[0];
}

//***************************************************
void cpubridge::notify_CPU_DECOUNTER_SET(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_decounter which, u16 valore)
{
	logger->log("notify_CPU_DECOUNTER_SET [%d] [%d]\n", (u8)which, valore);
	u8 buffer[4];
	buffer[0] = (u8)which;
	buffer[1] = (u8)((valore & 0xFF00) >> 8);
	buffer[2] = (u8)(valore & 0x00FF);
	rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_CPU_DECOUNTER_SET(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_decounter *out_which, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET);

	const u8 *p = (const u8*)msg.buffer;
	*out_which = (eCPUProgrammingCommand_decounter)p[0];
	*out_valore = ((u16)p[1] << 8) | p[2];
}

//***************************************************
void cpubridge::notify_CPU_ALL_DECOUNTER_VALUES(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *arrayDiAlmeno14Elementi, u32 sizeof_in_array)
{
	assert (sizeof_in_array >= 14 * sizeof(u16));
	logger->log("notify_CPU_ALL_DECOUNTER_VALUES\n");

	u16 buffer[14];
	memcpy(buffer, arrayDiAlmeno14Elementi, sizeof(u16) * 14);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_ALL_DECOUNTER_VALUES, handlerID, buffer, 14 * sizeof(u16));
}

//***************************************************
void cpubridge::translateNotify_CPU_ALL_DECOUNTER_VALUES(const rhea::thread::sMsg &msg, u16 *out_arrayDiAlmeno14Elementi, u32 sizeof_out_array)
{
	assert (sizeof_out_array >= 14 * sizeof(u16));
	assert(msg.what == CPUBRIDGE_NOTIFY_ALL_DECOUNTER_VALUES);
	memcpy(out_arrayDiAlmeno14Elementi, msg.buffer, sizeof(u16) * 14);
}

//***************************************************
void cpubridge::notify_EXTENDED_CONFIG_INFO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sExtendedCPUInfo *info)
{
	logger->log("notify_EXTENDED_CONFIG_INFO\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO, handlerID, info, sizeof(sExtendedCPUInfo));
}

//***************************************************
void cpubridge::translateNotify_EXTENDED_CONFIG_INFO(const rhea::thread::sMsg &msg, sExtendedCPUInfo *out_info)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO);
	memcpy (out_info, msg.buffer, sizeof(sExtendedCPUInfo));
}

//***************************************************
void cpubridge::notify_ATTIVAZIONE_MOTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec)
{
	logger->log("notify_ATTIVAZIONE_MOTORE\n");

	u16 buffer[4];
	buffer[0] = motore_1_10;
	buffer[1] = durata_dSec;
	buffer[2] = numRipetizioni;
	buffer[3] = pausaTraRipetizioni_dSec;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_ATTIVAZIONE_MOTORE, handlerID, buffer, 4);
}

//***************************************************
void cpubridge::translateNotify_ATTIVAZIONE_MOTORE(const rhea::thread::sMsg &msg, u8 *out_motore_1_10, u8 *out_durata_dSec, u8 *out_numRipetizioni, u8 *out_pausaTraRipetizioni_dSec)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_ATTIVAZIONE_MOTORE);

	const u8 *p = (const u8*)msg.buffer;
	*out_motore_1_10 = p[0];
	*out_durata_dSec = p[1];
	*out_numRipetizioni = p[2];
	*out_pausaTraRipetizioni_dSec = p[3];
}

//***************************************************
void cpubridge::notify_CALCOLA_IMPULSI_GRUPPO_STARTED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CALCOLA_IMPULSI_GRUPPO_STARTED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CALCOLA_IMPULSI_GRUPPO_STARTED, handlerID);
}

//***************************************************
void cpubridge::notify_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 stato, u16 valore)
{
	logger->log("notify_STATO_CALCOLO_IMPULSI_GRUPPO\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, valore);
	buffer[2] = stato;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_STATO_CALCOLO_IMPULSI_GRUPPO, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_STATO_CALCOLO_IMPULSI_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_stato, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_STATO_CALCOLO_IMPULSI_GRUPPO);

	const u8 *p = (const u8*)msg.buffer;
	*out_valore = rhea::utils::bufferReadU16(p);
	*out_stato = p[2];
}

//***************************************************
void cpubridge::notify_SET_FATTORE_CALIB_MOTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_motor motore, u16 valore)
{
	logger->log("notify_SET_FATTORE_CALIB_MOTORE\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, valore);
	buffer[2] = (u8)motore;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_SET_FATTORE_CALIB_MOTORE, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_motor *out_motore, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SET_FATTORE_CALIB_MOTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_valore = rhea::utils::bufferReadU16(p);
	*out_motore = (eCPUProgrammingCommand_motor)p[2];
}

//***************************************************
void cpubridge::notify_STATO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProgrammingCommand_statoGruppo stato)
{
	logger->log("notify_STATO_GRUPPO\n");

	u8 buffer[4];
	buffer[0] = (u8)stato;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_STATO_GRUPPO, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_STATO_GRUPPO(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_statoGruppo *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_STATO_GRUPPO);
	const u8 *p = (const u8*)msg.buffer;
	*out = (eCPUProgrammingCommand_statoGruppo)p[0];
}

//***************************************************
void cpubridge::notify_GET_TIME(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm, u8 ss)
{
	logger->log("notify_GET_TIME\n");

	u8 buffer[4];
	buffer[0] = hh;
	buffer[1] = mm;
	buffer[2] = ss;
	
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_GET_TIME, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_GET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_TIME);
	const u8 *p = (const u8*)msg.buffer;
	*out_hh = p[0];
	*out_mm = p[1];
	*out_ss = p[2];
}

//***************************************************
void cpubridge::notify_GET_DATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 year, u8 month, u8 day)
{
	logger->log("notify_GET_DATE\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, year);
	buffer[2] = month;
	buffer[3] = day;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_GET_DATE, handlerID, buffer, 4);
}

//***************************************************
void cpubridge::translateNotify_GET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_DATE);
	const u8 *p = (const u8*)msg.buffer;
	*out_year = rhea::utils::bufferReadU16(p);
	*out_month = p[2];
	*out_day = p[3];
}

//***************************************************
void cpubridge::notify_SET_TIME(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm, u8 ss)
{
	logger->log("notify_SET_TIME\n");

	u8 buffer[4];
	buffer[0] = hh;
	buffer[1] = mm;
	buffer[2] = ss;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_SET_TIME, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_SET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SET_TIME);
	const u8 *p = (const u8*)msg.buffer;
	*out_hh = p[0];
	*out_mm = p[1];
	*out_ss = p[2];
}

//***************************************************
void cpubridge::notify_SET_DATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 year, u8 month, u8 day)
{
	logger->log("notify_SET_DATE\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, year);
	buffer[2] = month;
	buffer[3] = day;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_SET_DATE, handlerID, buffer, 4);
}

//***************************************************
void cpubridge::translateNotify_SET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SET_DATE);
	const u8 *p = (const u8*)msg.buffer;
	*out_year = rhea::utils::bufferReadU16(p);
	*out_month = p[2];
	*out_day = p[3];
}

//***************************************************
void cpubridge::notify_CPU_POSIZIONE_MACINA(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 macina_1o2, u16 posizione)
{
	logger->log("notify_CPU_POSIZIONE_MACINA\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, posizione);
	buffer[2] = macina_1o2;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_POSIZIONE_MACINA, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_CPU_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_posizione)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_posizione = rhea::utils::bufferReadU16(p);
	*out_macina_1o2 = p[2];
}

//***************************************************
void cpubridge::notify_CPU_MOTORE_MACINA(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m)
{
	logger->log("notify_CPU_MOTORE_MACINA\n");

	u8 buffer[4];
	buffer[0] = macina_1o2;
	buffer[1] = (u8)m;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_MOTORE_MACINA, handlerID, buffer, 2);
}

//***************************************************
void cpubridge::translateNotify_CPU_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, eCPUProgrammingCommand_macinaMove *out_m)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_MOTORE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1o2 = p[0];
	*out_m = (eCPUProgrammingCommand_macinaMove)p[1];
}

//***************************************************
void cpubridge::notify_CPU_TEST_SELECTION(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d)
{
	logger->log("notify_CPU_TEST_SELECTION\n");

	u8 buffer[4];
	buffer[0] = selNum;
	buffer[1] = (u8)d;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_TEST_SELECTION, handlerID, buffer, 2);
}
//***************************************************
void cpubridge::translateNotify_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProgrammingCommand_testSelectionDevice *out_d)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_TEST_SELECTION);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_d = (eCPUProgrammingCommand_testSelectionDevice)p[1];
}

//***************************************************
void cpubridge::notify_NOMI_LINGE_CPU(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *strLingua1UTF16, const u16 *strLingua2UTF16)
{
	logger->log("notify_NOMI_LINGE_CPU\n");

	const u8 NUM_ELEM = (32+1) *2;
	u16 buffer[NUM_ELEM];
	memset(buffer, 0, sizeof(buffer));

	for (u8 i = 0; i < 32; i++)
	{
		if (strLingua1UTF16[i] == 0x0000)
			break;
		buffer[i] = strLingua1UTF16[i];
	}

	for (u8 i = 0; i < 32; i++)
	{
		if (strLingua2UTF16[i] == 0x0000)
			break;
		buffer[33 + i] = strLingua2UTF16[i];
	}
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_NOMI_LINGUE_CPU, handlerID, buffer, sizeof(buffer));
}

//***************************************************
void cpubridge::translateNotify_NOMI_LINGE_CPU(const rhea::thread::sMsg &msg, u16 *out_strLingua1UTF16, u16 *out_strLingua2UTF16)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_NOMI_LINGUE_CPU);
	const u16 *p = (const u16*)msg.buffer;
	
	out_strLingua1UTF16[0] = out_strLingua2UTF16[0] = 0x0000;
	for (u8 i = 0; i < 32; i++)
	{
		out_strLingua1UTF16[i] = p[i];
		if (p[i] == 0x0000)
			break;
	}

	p += 33;
	for (u8 i = 0; i < 32; i++)
	{
		out_strLingua2UTF16[i] = p[i];
		if (p[i] == 0x0000)
			break;
	}
}

//***************************************************
void cpubridge::notify_EVA_RESET_PARTIALDATA(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool result)
{
	logger->log("notify_EVA_RESET_PARTIALDATA\n");

	u8 buffer[4];
	if (result)
		buffer[0] = 0x01;
	else
		buffer[0] = 0x00;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_EVA_RESET_PARTIALDATA, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_EVA_RESET_PARTIALDATA(const rhea::thread::sMsg &msg, bool *out_result)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_EVA_RESET_PARTIALDATA);
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_result = true;
	else
		*out_result = false;	
}


//***************************************************
void cpubridge::notify_GET_VOLT_AND_TEMP(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 tCamera, u8 tBollitore, u8 tCappuccinatore, u16 voltaggio)
{
	logger->log("nnotify_GET_VOLT_AND_TEMP\n");

	u8 buffer[8];
	buffer[0] = tCamera;
	buffer[1] = tBollitore;
	buffer[2] = tCappuccinatore;
	rhea::utils::bufferWriteU16(&buffer[3], voltaggio);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_GET_VOLT_AND_TEMP, handlerID, buffer, 5);
}

//***************************************************
void cpubridge::translateNotify_GET_VOLT_AND_TEMP(const rhea::thread::sMsg &msg, u8 *out_tCamera, u8 *out_tBollitore, u8 *out_tCappuccinatore, u16 *out_voltaggio)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_GET_VOLT_AND_TEMP);
	const u8 *p = (const u8*)msg.buffer;
	*out_tCamera = p[0];
	*out_tBollitore = p[1];
	*out_tCappuccinatore = p[2];
	*out_voltaggio = rhea::utils::bufferReadU16(&p[3]);
}

//***************************************************
void cpubridge::notify_GET_OFF_REPORT(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 indexNum, u8 lastIndexNum, const sCPUOffSingleEvent *offs, u8 numOffs)
{
	if (numOffs == 0)
		return;

	logger->log("notify_GET_OFF_REPORT\n");

	u8 buffer[256];
	u16 ct = 0;
	buffer[ct++] = indexNum;
	buffer[ct++] = lastIndexNum;
	buffer[ct++] = numOffs;

	for (u8 i = 0; i < numOffs; i++)
	{
		buffer[ct++] = offs[i].codice;
		buffer[ct++] = offs[i].tipo;
		buffer[ct++] = offs[i].ora;
		buffer[ct++] = offs[i].minuto;
		buffer[ct++] = offs[i].giorno;
		buffer[ct++] = offs[i].mese;
		buffer[ct++] = offs[i].anno;
		buffer[ct++] = offs[i].stato;
	}

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_GET_OFF_REPORT, handlerID, buffer, ct);
}

//***************************************************
void cpubridge::translateNotify_GET_OFF_REPORT(const rhea::thread::sMsg &msg, u8 *out_indexNum, u8 *out_lastIndexNum, u8 *out_numOffs, sCPUOffSingleEvent *out, u32 sizeofOut)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_GET_OFF_REPORT);
	const u8 *p = (const u8*)msg.buffer;
	u16 ct = 0;
	*out_indexNum = p[ct++];
	*out_lastIndexNum = p[ct++];
	*out_numOffs = p[ct++];

	u16 nMaxInOutBuffer = sizeofOut / sizeof(sCPUOffSingleEvent);
	if (nMaxInOutBuffer > 0xff)
		nMaxInOutBuffer = 0xff;

	if (*out_numOffs > nMaxInOutBuffer)
		*out_numOffs = (u8)nMaxInOutBuffer;

	for (u8 i = 0; i < (*out_numOffs); i++)
	{
		out[i].codice = p[ct++];
		out[i].tipo = p[ct++];
		out[i].ora = p[ct++];
		out[i].minuto = p[ct++];
		out[i].giorno = p[ct++];
		out[i].mese = p[ct++];
		out[i].anno = p[ct++];
		out[i].stato = p[ct++];
	}
}

//***************************************************
void cpubridge::notify_GET_LAST_FLUX_INFORMATION(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 lastFlux, u16 lastGrinderPosition)
{
	logger->log("notify_GET_LAST_FLUX_INFORMATION\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(&buffer[0], lastFlux);
	rhea::utils::bufferWriteU16(&buffer[2], lastGrinderPosition);

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_GET_LAST_FLUX_INFORMATION, handlerID, buffer, 4);
}

//***************************************************
void cpubridge::translateNotify_GET_LAST_FLUX_INFORMATION(const rhea::thread::sMsg &msg, u16 *out_lastFlux, u16 *out_lastGrinderPosition)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_GET_LAST_FLUX_INFORMATION);
	const u8 *p = (const u8*)msg.buffer;
	*out_lastFlux = rhea::utils::bufferReadU16(&p[0]);
	*out_lastGrinderPosition = rhea::utils::bufferReadU16(&p[2]);
}

//***************************************************
void cpubridge::notify_CPU_STRING_VERSION_AND_MODEL(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *utf16_msg)
{
	logger->log("notify_CPU_STRING_VERSION_AND_MODEL\n");

	u32 n = rhea::string::utf16::lengthInBytes(utf16_msg);
	if (n > 0)
		rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_GET_CPU_STRING_MODEL_AND_VER, handlerID, utf16_msg, (n+1)*2);
}

//***************************************************
void cpubridge::translateNotify_CPU_STRING_VERSION_AND_MODEL(const rhea::thread::sMsg &msg, u16 *out_utf16msg, u32 sizeOfOutUTF16MsgInBytes)
{
	assert(msg.what == CPUBRIDGE_NOTITFY_GET_CPU_STRING_MODEL_AND_VER);
	const u16 *p = (const u16*)msg.buffer;

	const u32 n = rhea::string::utf16::lengthInBytes(p);
	const u32 nBytesNeeded = (n+1) * 2;
	if (sizeOfOutUTF16MsgInBytes >= nBytesNeeded)
		memcpy(out_utf16msg, p, nBytesNeeded);
	else
	{
		memcpy(out_utf16msg, p, sizeOfOutUTF16MsgInBytes);
		out_utf16msg[(sizeOfOutUTF16MsgInBytes / 2) - 1] = 0;
	}

}

//***************************************************
void cpubridge::notify_CPU_START_MODEM_TEST(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_START_MODEM_TEST\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_CPU_START_MODEM_TEST, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_CPU_EVA_RESET_TOTALS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_EVA_RESET_TOTALS\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTITFY_CPU_EVA_RESET_TOTALS, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm)
{
	logger->log("notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE\n");

	u8 buffer[4];
	buffer[0] = hh;
	buffer[1] = mm;

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_GET_TIME_LAVSAN_CAPPUCINATORE, handlerID, buffer, 2);
}

//***************************************************
void cpubridge::translateNotify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_TIME_LAVSAN_CAPPUCINATORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_hh = p[0];
	*out_mm = p[1];
}

//***************************************************
void cpubridge::notify_START_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_START_TEST_ASSORBIMENTO_GRUPPO\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 fase, u8 esito, const u16 *results)
{
	logger->log("notify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO\n");

	u8 buffer[32];
	buffer[0] = fase;
	buffer[1] = esito;
	for (u8 i=0; i<12; i++)
		rhea::utils::bufferWriteU16(&buffer[2 +i*2], results[i]);

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_GETSTATUS_TEST_ASSORBIMENTO_GRUPPO, handlerID, buffer, 26);
}

//***************************************************
void cpubridge::translateNotify_GET_STATUS_TEST_ASSORBIMENTO_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_fase, u8 *out_esito, u16 *out_12results)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GETSTATUS_TEST_ASSORBIMENTO_GRUPPO);
	const u8 *p = (const u8*)msg.buffer;
	*out_fase = p[0];
	*out_esito = p[1];
	for (u8 i = 0; i < 12; i++)
		out_12results[i] = rhea::utils::bufferReadU16(&p[2 + i * 2]);
}

//***************************************************
void cpubridge::notify_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 fase, u8 esito, u16 reportUP, u16 reportDOWN)
{
	logger->log("notify_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE\n");

	u8 buffer[32];
	buffer[0] = fase;
	buffer[1] = esito;
	rhea::utils::bufferWriteU16(&buffer[2], reportUP);
	rhea::utils::bufferWriteU16(&buffer[4], reportDOWN);

	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_GETSTATUSTEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, buffer, 6);
}

//***************************************************
void cpubridge::translateNotify_GET_STATUS_TEST_ASSORBIMENTO_MOTORIDUTTORE(const rhea::thread::sMsg &msg, u8 *out_fase, u8 *out_esito, u16 *out_reportUP, u16 *out_reportDOWN)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GETSTATUSTEST_ASSORBIMENTO_MOTORIDUTTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_fase = p[0];
	*out_esito = p[1];
	*out_reportUP = rhea::utils::bufferReadU16(&p[2]);
	*out_reportDOWN = rhea::utils::bufferReadU16(&p[4]);
}


//***************************************************
void cpubridge::notify_CPU_MILKER_VER (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const char *ver)
{
	logger->log("notify_CPU_MILKER_VER\n");

	const u32 len = (u32)strlen(ver);
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_MILKER_VER, handlerID, ver, len+1);
}

//***************************************************
void cpubridge::translateNotify_CPU_MILKER_VER(const rhea::thread::sMsg &msg, char *out_ver, u32 sizeofOutVer)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_MILKER_VER);

	if (sizeofOutVer >= msg.bufferSize)
		memcpy(out_ver, msg.buffer, msg.bufferSize);
	else
	{
		memcpy(out_ver, msg.buffer, sizeofOutVer - 1);
		out_ver[sizeofOutVer] = 0;
	}
}













//***************************************************
void cpubridge::ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION, (u32)selNumber);
}

//***************************************************
void cpubridge::translate_CPU_START_SELECTION (const rhea::thread::sMsg &msg, u8 *out_selNumber)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION);
    *out_selNumber = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::ask_CPU_STOP_SELECTION (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION, (u32)0);
}

//***************************************************
void cpubridge::ask_CPU_SEND_BUTTON(const sSubscriber &from, u8 buttonNum)
{
    rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM, (u32)buttonNum);
}

//***************************************************
void cpubridge::translate_CPU_SEND_BUTTON(const rhea::thread::sMsg &msg, u8 *out_buttonNum)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM);
    *out_buttonNum = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::ask_CPU_KEEP_SENDING_BUTTON_NUM(const sSubscriber &from, u8 buttonNum)
{
    rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_KEEP_SENDING_BUTTON_NUM, (u32)buttonNum);
}

//***************************************************
void cpubridge::translate_CPU_KEEP_SENDING_BUTTON_NUM(const rhea::thread::sMsg &msg, u8 *out_buttonNum)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_KEEP_SENDING_BUTTON_NUM);
    *out_buttonNum = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::ask_CPU_QUERY_RUNNING_SEL_STATUS(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_FULLSTATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_INI_PARAM(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_AVAIL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_PRICES (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_LCD_MESSAGE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_CURRENT_CREDIT(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_STATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE, handlerID);
}

//***************************************************
void cpubridge::ask_READ_DATA_AUDIT(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT, handlerID);
}

//***************************************************
void cpubridge::ask_READ_VMCDATAFILE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE, handlerID);
}

//***************************************************
void cpubridge::ask_WRITE_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const u8* const srcFullFileNameAndPath)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE, handlerID, srcFullFileNameAndPath, rhea::string::utf8::lengthInBytes(srcFullFileNameAndPath)+1);
}

//***************************************************
void cpubridge::translate_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_srcFullFileNameAndPath, u32 sizeOfOut)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE);
	u32 n = msg.bufferSize;
	if (n > sizeOfOut)
	{
		DBGBREAK;
		n = sizeOfOut;
	}

	memcpy(out_srcFullFileNameAndPath, msg.buffer, n);
}


//***************************************************
void cpubridge::ask_WRITE_PARTIAL_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const u8 *buffer64byte, u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset)
{
	u8 buffer[VMCDATAFILE_BLOCK_SIZE_IN_BYTE + 3];
	memcpy(buffer, buffer64byte, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	buffer[VMCDATAFILE_BLOCK_SIZE_IN_BYTE] = blocco_n_di;
	buffer[VMCDATAFILE_BLOCK_SIZE_IN_BYTE+1] = tot_num_blocchi;
	buffer[VMCDATAFILE_BLOCK_SIZE_IN_BYTE+2] = blockNumOffset;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE, handlerID, buffer, sizeof(buffer));
}

//***************************************************
void cpubridge::translate_PARTIAL_WRITE_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_buffer64byte, u8 *out_blocco_n_di, u8 *out_tot_num_blocchi, u8 *out_blockNumOffset)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE);

	const u8 *p = (const u8*)msg.buffer;
	memcpy (out_buffer64byte, msg.buffer, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	*out_blocco_n_di = p[VMCDATAFILE_BLOCK_SIZE_IN_BYTE];
	*out_tot_num_blocchi = p[VMCDATAFILE_BLOCK_SIZE_IN_BYTE+1];
	*out_blockNumOffset = p[VMCDATAFILE_BLOCK_SIZE_IN_BYTE+2];
}

//***************************************************
void cpubridge::ask_CPU_VMCDATAFILE_TIMESTAMP(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP, handlerID);
}


//***************************************************
void cpubridge::ask_WRITE_CPUFW(const sSubscriber &from, u16 handlerID, const u8* const srcFullFileNameAndPath)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_CPUFW, handlerID, srcFullFileNameAndPath, rhea::string::utf8::lengthInBytes(srcFullFileNameAndPath) + 1);
}

//***************************************************
void cpubridge::translate_WRITE_CPUFW(const rhea::thread::sMsg &msg, u8 *out_srcFullFileNameAndPath, u32 sizeOfOut)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_WRITE_CPUFW);
	u32 n = msg.bufferSize;
	if (n > sizeOfOut)
	{
		DBGBREAK;
		n = sizeOfOut;
	}

	memcpy(out_srcFullFileNameAndPath, msg.buffer, n);
}


//***************************************************
void cpubridge::ask_CPU_PROGRAMMING_CMD (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand cmd, const u8 *optionalData, u32 sizeOfOptionalData)
{
	assert(sizeOfOptionalData < 31);
    u8 otherData[32];
    otherData[0] = (u8)cmd;
	if (optionalData != NULL && sizeOfOptionalData>0)
		memcpy(&otherData[1], optionalData, sizeOfOptionalData);
    rhea::thread::pushMsg (from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD, handlerID, otherData, 1+ sizeOfOptionalData);
}

//***************************************************
void cpubridge::translate_CPU_PROGRAMMING_CMD(const rhea::thread::sMsg &msg, eCPUProgrammingCommand *out, const u8 **out_optionalData)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD);

    const u8 *p = (const u8*)msg.buffer;
    *out = (eCPUProgrammingCommand)p[0];
	*out_optionalData = &p[1];
}

//***************************************************
void cpubridge::ask_CPU_SET_DECOUNTER (const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_decounter which, u16 valore)
{
	u8 otherData[3];
	otherData[0] = (u8)which;
	otherData[1] = (u8)((valore & 0xFF00) >> 8);
	otherData[2] = (u8)(valore & 0x00FF);
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_DECOUNTER (const rhea::thread::sMsg &msg, eCPUProgrammingCommand_decounter *out_which, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER);
	const u8 *p = (const u8*)msg.buffer;
	*out_which = (eCPUProgrammingCommand_decounter)p[0];
	*out_valore = ((u16)p[1] << 8) | p[2];
}

//***************************************************
void cpubridge::ask_CPU_GET_ALL_DECOUNTER_VALUES(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_ALL_DECOUNTER_VALUES, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_EXTENDED_CONFIG_INFO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_ATTIVAZIONE_MOTORE(const sSubscriber &from, u16 handlerID, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec)
{
	u8 otherData[4];
	otherData[0] = motore_1_10;
	otherData[1] = durata_dSec;
	otherData[2] = numRipetizioni;
	otherData[3] = pausaTraRipetizioni_dSec;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_ATTIVAZIONE_MOTORE, handlerID, otherData, 4);
}

//***************************************************
void cpubridge::translate_CPU_ATTIVAZIONE_MOTORE(const rhea::thread::sMsg &msg, u8 *out_motore_1_10, u8 *out_durata_dSec, u8 *out_numRipetizioni, u8 *out_pausaTraRipetizioni_dSec)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_ATTIVAZIONE_MOTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_motore_1_10 = p[0];
	*out_durata_dSec = p[1];
	*out_numRipetizioni = p[2];
	*out_pausaTraRipetizioni_dSec = p[3];
}

//***************************************************
void cpubridge::ask_CPU_CALCOLA_IMPULSI_GRUPPO(const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 totalePesata_dGrammi)
{
	assert(macina_1o2 == 1 || macina_1o2 == 2);

	u8 otherData[4];
	otherData[0] = macina_1o2;
	rhea::utils::bufferWriteU16(&otherData[1], totalePesata_dGrammi);
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_CALCOLA_IMPULSI_GRUPPO(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_totalePesata_dGrammi)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1o2 = p[0];
	*out_totalePesata_dGrammi = rhea::utils::bufferReadU16(&p[1]);
}




//***************************************************
void cpubridge::ask_CPU_GET_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_CALCOLO_IMPULSI_GRUPPO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_SET_FATTORE_CALIB_MOTORE(const sSubscriber &from, u16 handlerID, eCPUProgrammingCommand_motor motore, u16 valoreGr)
{
	u8 otherData[4];
	otherData[0] = (u8)motore;
	rhea::utils::bufferWriteU16(&otherData[1], valoreGr);
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProgrammingCommand_motor *out_motore, u16 *out_valoreGr)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_motore = (eCPUProgrammingCommand_motor)p[0];
	*out_valoreGr = rhea::utils::bufferReadU16(&p[1]);
}

//***************************************************
void cpubridge::ask_CPU_GET_STATO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_GRUPPO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_TIME(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_TIME, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_DATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_DATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_VOLT_AND_TEMP(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_VOLT_AND_TEMP, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_OFF_REPORT(const sSubscriber &from, u16 handlerID, u8 indexNum)
{
	u8 otherData[4];
	otherData[0] = indexNum;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_OFF_REPORT, handlerID, otherData, 1);
}

//***************************************************
void cpubridge::translate_CPU_GET_OFF_REPORT(const rhea::thread::sMsg &msg, u8 *out_indexNum)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_GET_OFF_REPORT);
	const u8 *p = (const u8*)msg.buffer;
	*out_indexNum = p[0];
}

//***************************************************
void cpubridge::ask_CPU_GET_LAST_FLUX_INFORMATION(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_FLUX_INFORMATION, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_SHOW_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SHOW_STR_VERSION_AND_MODEL, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_STR_VERSION_AND_MODEL, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_DA3SYNC(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_DA3_SYNC, 0, NULL, 0);
}


//***************************************************
void cpubridge::ask_CPU_START_MODEM_TEST(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_START_MODEM_TEST, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_SET_TIME(const sSubscriber &from, u16 handlerID, u8 hh, u8 mm, u8 ss)
{
	u8 otherData[4];
	otherData[0] = hh;
	otherData[1] = mm;
	otherData[2] = ss;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_TIME, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_TIME(const rhea::thread::sMsg &msg, u8 *out_hh, u8 *out_mm, u8 *out_ss)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_TIME);
	const u8 *p = (const u8*)msg.buffer;
	*out_hh = p[0];
	*out_mm = p[1];
	*out_ss = p[2];
}


//***************************************************
void cpubridge::ask_CPU_SET_DATE(const sSubscriber &from, u16 handlerID, u16 year, u8 month, u8 day)
{
	u8 otherData[4];
	otherData[0] = (u8)(year - 2000);
	otherData[1] = month;
	otherData[2] = day;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_DATE, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_DATE(const rhea::thread::sMsg &msg, u16 *out_year, u8 *out_month, u8 *out_day)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_DATE);
	const u8 *p = (const u8*)msg.buffer;
	*out_year = (u16)p[0] + 2000;
	*out_month = p[1];
	*out_day = p[2];
}


//***************************************************
void cpubridge::ask_CPU_GET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2)
{
	if (macina_1o2 != 2)
		macina_1o2 = 1;

	u8 otherData[1];
	otherData[0] = macina_1o2;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA, handlerID, otherData, 1);
}

//***************************************************
void cpubridge::translate_CPU_GET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1o2 = p[0];
}

//***************************************************
void cpubridge::ask_CPU_SET_MOTORE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, eCPUProgrammingCommand_macinaMove m)
{
	if (macina_1o2 != 2)
		macina_1o2 = 1;

	u8 otherData[2];
	otherData[0] = macina_1o2;
	otherData[1] = (u8)m;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_SET_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, eCPUProgrammingCommand_macinaMove *out_m)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1o2 = p[0];
	*out_m = (eCPUProgrammingCommand_macinaMove)p[1];
}

//***************************************************
void cpubridge::ask_CPU_SET_POSIZIONE_MACINA(const sSubscriber &from, u16 handlerID, u8 macina_1o2, u16 target)
{
	if (macina_1o2 != 2)
		macina_1o2 = 1;

	u8 otherData[4];
	rhea::utils::bufferWriteU16(otherData, target);
	otherData[2] = macina_1o2;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_target)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_target = rhea::utils::bufferReadU16(p);
	*out_macina_1o2 = p[2];
}

//***************************************************
void cpubridge::ask_CPU_TEST_SELECTION(const sSubscriber &from, u16 handlerID, u8 selNum, eCPUProgrammingCommand_testSelectionDevice d)
{
	u8 otherData[4];
	otherData[0] = selNum;
	otherData[1] = (u8)d;
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProgrammingCommand_testSelectionDevice *out_d)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_d = (eCPUProgrammingCommand_testSelectionDevice)p[1];
}

//***************************************************
void cpubridge::ask_CPU_GET_NOMI_LINGE_CPU(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_NOMI_LINGUE_CPU, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_DISINTALLAZIONE(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_DISINSTALLAZIONE, 0, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_RICARICA_FASCIA_ORARIA_FREEVEND(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_RICARICA_FASCIA_ORARIA_FV, 0, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_EVA_RESET_PARTIALDATA(const sSubscriber &from, u16 handlerID UNUSED_PARAM)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_PARTIALDATA, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_EVA_RESET_TOTALS(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_TOTALS, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_TIME_NEXT_LAVSAN_CAPPUCC, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_MILKER_VER(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_MILKER_VER, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (const sSubscriber &from, u8 selNum, u16 price, eGPUPaymentType paymentType)
{
	u8 otherData[8];
	otherData[0] = selNum;
	otherData[1] = (u8)paymentType;
	rhea::utils::bufferWriteU16(&otherData[2], price);	

	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED, (u32)0, otherData, 4);
}
//***************************************************
void cpubridge::translate_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED(const rhea::thread::sMsg &msg, u8 *out_selNum, u16 *out_price, eGPUPaymentType *out_paymentType)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_paymentType = (eGPUPaymentType)p[1];
	*out_price = rhea::utils::bufferReadU16 (&p[2]);
}

