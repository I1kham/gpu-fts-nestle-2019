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
    cpubridge_helper_folder_create("current/gpu", logger);
    cpubridge_helper_folder_create("last_installed", logger);
    cpubridge_helper_folder_create("/last_installed/da3", logger);
    cpubridge_helper_folder_create("last_installed/cpu", logger);
    cpubridge_helper_folder_create("temp", logger);
    cpubridge_helper_folder_create("autoUpdate", logger);

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
    FILE *f = rhea::fs::fileOpenForReadBinary((const u8*)s);
    if (NULL == f)
        return;
    out->readFromFile(f);
    rhea::fs::fileClose(f);
}

//***************************************************
bool cpubridge::saveVMCDataFileTimeStamp(const sCPUVMCDataFileTimeStamp &ts)
{
    char s[512];
    sprintf_s(s, sizeof(s), "%s/current/da3/vmcDataFile.timestamp", rhea::getPhysicalPathToAppFolder());
    FILE *f = rhea::fs::fileOpenForWriteBinary((const u8*)s);
    if (NULL == f)
        return false;
    ts.writeToFile(f);
    rhea::fs::fileClose(f);
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
		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();

	rhea::thread::deleteMsgQ (chToThreadR, chToThreadW);
	return 1;
}

//*******************************************
bool cpubridge::copyFileInAutoupdateFolder (const u8 *fullSrcFilePathAndName)
{
    u8 s[512];
    rhea::string::utf8::spf (s, sizeof(s), "%s/autoUpdate", rhea::getPhysicalPathToAppFolder());
    return rhea::fs::fileCopyAndKeepSameName (fullSrcFilePathAndName, s);
}

//*******************************************
bool cpubridge::copyFileInAutoupdateFolder (const u8 *fullSrcFilePathAndName, const char *dstFileName)
{
    u8 s[512];
    rhea::string::utf8::spf (s, sizeof(s), "%s/autoUpdate/%s", rhea::getPhysicalPathToAppFolder(), dstFileName);
    return rhea::fs::fileCopy (fullSrcFilePathAndName, s);
}


/***************************************************
 * ritorna 0 se out_buffer non è abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 */
u8 cpubridge_buildMsg (cpubridge::eCPUCommand command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	//calcolo della dimensione totale
	if (sizeOfOutBuffer < (u32)(4 + sizeOfOptionaData))
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
u8 cpubridge::buildMsg_checkStatus_B (u8 keyPressed, u8 langErrorCode, bool forceJUG, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[8];
	u8 ct = 0;
	optionalData[ct++] = keyPressed;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = langErrorCode;

	//2021-06-10	abbiamo aggiunto 1 byte che funziona a mo' di bitmask
	u8 flag = 0;
	if (forceJUG)
		flag |= CPU_MSG_B_BYTE6_FLAG_FORCE_JUG;
	optionalData[ct++] = flag;

	return cpubridge_buildMsg (cpubridge::eCPUCommand::checkStatus_B, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startSelectionWithPaymentAlreadyHandledByGPU_V (u8 selNum, u16 prezzo, ePaymentMode paymentMode, eGPUPaymentType paymentType, bool bForceJUG UNUSED_PARAM, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[8];
	u8 ct = 0;
	optionalData[ct++] = selNum;
	rhea::utils::bufferWriteU16_LSB_MSB (&optionalData[ct], prezzo);
	ct += 2;
	optionalData[ct++] = (u8)paymentMode;
	optionalData[ct++] = (u8)paymentType;

	return cpubridge_buildMsg (cpubridge::eCPUCommand::startSelWithPaymentAlreadyHandled_V, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_richiestaNomeSelezioneDiCPU_d (u8 selNum, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg (cpubridge::eCPUCommand::getNomeSelezioneCPU_d, &selNum, 1, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_setDecounter (eCPUProg_decounter which, u16 valore, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[3];
	optionalData[0] = (u8)which;
	optionalData[1] = (u8)(valore & 0x00FF);			//LSB
	optionalData[2] = (u8)((valore & 0xFF00) >> 8);		//MSB
	
	return buildMsg_Programming(eCPUProgrammingCommand::setDecounter, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getAllDecounterValues(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getAllDecounterValues, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_calcolaImpulsiGruppo_AA (u8 macina_1to4, u16 totalePesata_dGrammi, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];

	optionalData[0] = 10 + macina_1to4;
	rhea::utils::bufferWriteU16_LSB_MSB(&optionalData[1], totalePesata_dGrammi);
	return buildMsg_Programming (eCPUProgrammingCommand::calcolaImpulsiMacina, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoCalcoloImpulsiGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getStatoCalcoloImpulsi, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getTime(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getTime, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_stopJug(u8* out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::stop_jug, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getJugCurrentRepetition(u8* out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::get_jug_current_repetition, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getDate(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getDate, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setTime(u8 *out_buffer, u8 sizeOfOutBuffer, u8 hh, u8 mm, u8 ss)
{
	u8 optionalData[4];
	optionalData[0] = hh;
	optionalData[1] = mm;
	optionalData[2] = ss;
	return buildMsg_Programming(eCPUProgrammingCommand::setTime, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setDate(u8 *out_buffer, u8 sizeOfOutBuffer, u16 year, u8 month, u8 day)
{
	u8 optionalData[4];
	optionalData[0] = (u8)(year-2000);
	optionalData[1] = month;
	optionalData[2] = day;
	return buildMsg_Programming(eCPUProgrammingCommand::setDate, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getPosizioneMacina_AA(u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1to4)
{
	if (macina_1to4<1 || macina_1to4>4)
		macina_1to4 = 1;
	macina_1to4 += 10;	//le macine vanno da 11 a 20

	u8 optionalData[2];
	optionalData[0] = macina_1to4;
	return buildMsg_Programming (eCPUProgrammingCommand::getPosizioneMacina, optionalData, 1, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setMotoreMacina_AA (u8 *out_buffer, u8 sizeOfOutBuffer, u8 macina_1to4, eCPUProg_macinaMove m)
{
	if (macina_1to4<1 || macina_1to4>4)
		macina_1to4 = 1;
	macina_1to4 += 10;	//le macine vanno da 11 a 20

	u8 optionalData[2];
	optionalData[0] = macina_1to4;
	optionalData[1] = (u8)m;
	return buildMsg_Programming(eCPUProgrammingCommand::setMotoreMacina, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_testSelection (u8 *out_buffer, u8 sizeOfOutBuffer, u8 selNum, eCPUProg_testSelectionDevice d)
{
	u8 optionalData[4];
	optionalData[0] = selNum;
	optionalData[1] = (u8)d;

	return buildMsg_Programming(eCPUProgrammingCommand::testSelezione, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getNomiLingueCPU(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getNomiLinguaCPU, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_disintallazione(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::disinstallazione, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_ricaricaFasciaOrariaFreevend(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::ricaricaFasciaOrariaFV, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_EVAresetPartial(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::EVAresetPartial, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_EVAresetTotals(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::EVAresetTotals, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_getVoltAndTemp(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getVoltAndTemp, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getCPUOFFReportDetails(u8 *out_buffer, u8 sizeOfOutBuffer, u8 indexNum)
{
	u8 optionalData[4];
	optionalData[0] = indexNum;
	return buildMsg_Programming(eCPUProgrammingCommand::getCPUOFFReportDetails, optionalData, 1, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getLastFluxInformation(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getLastFluxInformation, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getCPUStringVersionAndModel(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getStringVersionAndModel, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startModemTest(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::startModemTest, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::startTestAssorbGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_startTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::startTestAssorbMotoriduttore, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_attivazioneMotore(u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	optionalData[0] = motore_1_10;
	optionalData[1] = durata_dSec;
	optionalData[2] = numRipetizioni;
	optionalData[3] = pausaTraRipetizioni_dSec;

	return buildMsg_Programming(eCPUProgrammingCommand::attivazioneMotore, optionalData, 4, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setFattoreCalibMotore(eCPUProg_motor motore, u16 valoreInGr, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	optionalData[0] = (u8)motore;
	rhea::utils::bufferWriteU16_LSB_MSB(&optionalData[1], valoreInGr);
	return buildMsg_Programming(eCPUProgrammingCommand::setFattoreCalibrazioneMotore, optionalData, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getExtendedConfigInfo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand::getExtendedConfigInfo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getStatoGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoTestAssorbimentoGruppo(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getStatusTestAssorbGruppo, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getStatoTestAssorbimentoMotoriduttore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getStatusTestAssorbMotoriduttore, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_initialParam_C(u8 gpuVersionMajor, u8 gpuVersionMinor, u8 gpuVersionBuild, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	u8 ct = 0;

	optionalData[ct++] = gpuVersionMajor;
	optionalData[ct++] = gpuVersionMinor;
	optionalData[ct++] = gpuVersionBuild;

	return cpubridge_buildMsg (cpubridge::eCPUCommand::initialParam_C, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_restart_U(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg (cpubridge::eCPUCommand::restart, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_readDataAudit (u8 *out_buffer, u8 sizeOfOutBuffer)
{
    u8 optionalData[2] = {0,0};
    return cpubridge_buildMsg (cpubridge::eCPUCommand::readDataAudit, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_readVMCDataFile(u8 blockNum, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[2] = { blockNum, 0 };
	return cpubridge_buildMsg(cpubridge::eCPUCommand::readVMCDataFile, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_writeVMCDataFile(const u8 *buffer64yteLettiDalFile, u8 blockNum, u8 totNumBlocks, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[VMCDATAFILE_BLOCK_SIZE_IN_BYTE + 2];
	optionalData[0] = blockNum;
	optionalData[1] = totNumBlocks;
	memcpy(&optionalData[2], buffer64yteLettiDalFile, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	return cpubridge_buildMsg(cpubridge::eCPUCommand::writeVMCDataFile, optionalData, sizeof(optionalData), out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_requestPriceHoldingPriceList (u8 firstPrice, u8 numPrices, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[2];
	optionalData[0] = firstPrice;
	optionalData[1] = numPrices;
	return cpubridge_buildMsg(cpubridge::eCPUCommand::requestPriceHoldingPriceList, optionalData, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_writePartialVMCDataFile(const u8 *buffer64byte, u8 blocco_n_di, u8 tot_num_blocchi, u8 blockNumOffset, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[VMCDATAFILE_BLOCK_SIZE_IN_BYTE + 3];
	optionalData[0] = blocco_n_di;
	optionalData[1] = tot_num_blocchi;
	optionalData[2] = blockNumOffset;
	memcpy(&optionalData[3], buffer64byte, VMCDATAFILE_BLOCK_SIZE_IN_BYTE);
	return cpubridge_buildMsg(cpubridge::eCPUCommand::writePartialVMCDataFile, optionalData, sizeof(optionalData), out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getVMCDataFileTimeStamp (u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand::getVMCDataFileTimeStamp, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getMilkerVer(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg(cpubridge::eCPUCommand::getMilkerVer, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_Snack (eSnackCommand cmd, const u8 *optionalDataIN, u32 sizeOfOptionalDataIN, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	assert(sizeOfOptionalDataIN < 31);
    u8 optionalData[32];
    optionalData[0] = (u8)cmd;
	if (NULL != optionalDataIN && sizeOfOptionalDataIN > 0)
		memcpy(&optionalData[1], optionalDataIN, sizeOfOptionalDataIN);
    return cpubridge_buildMsg (cpubridge::eCPUCommand::snackCommand, optionalData, 1+ sizeOfOptionalDataIN, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_Snack_status_0x03 (u8 *out_buffer, u8 sizeOfOutBuffer)					{ return buildMsg_Snack (eSnackCommand::machineStatus, NULL, 0, out_buffer, sizeOfOutBuffer); }
u8 cpubridge::buildMsg_Snack_enterProg_0x04 (u8* out_buffer, u8 sizeOfOutBuffer)				{ return buildMsg_Snack (eSnackCommand::enterProg, NULL, 0, out_buffer, sizeOfOutBuffer); }
u8 cpubridge::buildMsg_Snack_exitProg_0x05 (u8* out_buffer, u8 sizeOfOutBuffer)					{ return buildMsg_Snack (eSnackCommand::exitProg, NULL, 0, out_buffer, sizeOfOutBuffer); }

//***************************************************
u8 cpubridge::buildMsg_Programming (eCPUProgrammingCommand cmd, const u8 *optionalDataIN, u32 sizeOfOptionalDataIN, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	assert(sizeOfOptionalDataIN < 31);
    u8 optionalData[32];
    optionalData[0] = (u8)cmd;
	if (NULL != optionalDataIN && sizeOfOptionalDataIN > 0)
		memcpy(&optionalData[1], optionalDataIN, sizeOfOptionalDataIN);
    return cpubridge_buildMsg (cpubridge::eCPUCommand::programming, optionalData, 1+ sizeOfOptionalDataIN, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getTimeNextLavaggioSanCappuccinatore(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getTimeNextLavaggioCappuccinatore, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getBuzzerStatus (u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::get_cpu_buzzer_status, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//***************************************************
u8 cpubridge::buildMsg_getLastGrinderSpeed (u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getLastGrinderSpeed, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getCupSensorLiveValue (u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return buildMsg_Programming(eCPUProgrammingCommand::getCupSensorLiveValue, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_activateCPUBuzzer (u8 numRepeat, u8 beepLen_dSec, u8 pausaTraUnBeepELAltro_dSec, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	if (numRepeat>15) numRepeat= 15;
	if (beepLen_dSec>15) beepLen_dSec= 15;
	if (pausaTraUnBeepELAltro_dSec>15) pausaTraUnBeepELAltro_dSec= 15;

	u8 payload[2];
	payload[0] = (numRepeat | 0x80);
	payload[1] = beepLen_dSec | (pausaTraUnBeepELAltro_dSec << 4);
	return buildMsg_Programming(eCPUProgrammingCommand::activate_cpu_buzzer, payload, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_notifyEndOfGrinderCleaningProcedure (u8 grinder1toN, u8* out_buffer, u8 sizeOfOutBuffer)
{
	u8 payload[2];
    payload[0] = grinder1toN;
	return buildMsg_Programming(eCPUProgrammingCommand::notify_end_of_grinder_cleaning_proc, payload, 1, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_scivoloBrewmatic (u8 perc0_100, u8* out_buffer, u8 sizeOfOutBuffer)
{
	if (perc0_100 > 100)
		perc0_100 = 100;

	u8 payload[2];
	payload[0] = perc0_100;
	return buildMsg_Programming(eCPUProgrammingCommand::scivolo_brewmatic, payload, 1, out_buffer, sizeOfOutBuffer);
}

u8 cpubridge::buildMsg_askMessageFromLanguageTable (u8 tableID, u8 msgRowNum, u8 language1or2, u8* out_buffer, u32 sizeOfOutBuffer)
{
	assert (language1or2==1 || language1or2==2);

	u8 payload[4];
	payload[0] = tableID;
	payload[1] = msgRowNum;
	payload[2] = language1or2;
	return buildMsg_Programming(eCPUProgrammingCommand::ask_msg_from_table_language, payload, 3, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_setSelectionParam (u8 selNum1ToN, eSelectionParam whichParam, u16 paramValue, u8* out_buffer, u32 sizeOfOutBuffer)
{
	if (selNum1ToN<1 || selNum1ToN > NUM_MAX_SELECTIONS)
	{
		DBGBREAK;
		return 0;
	}

	const u8 paramID = static_cast<u8>(whichParam);
	switch (whichParam)
	{
	default:
		DBGBREAK;
		return 0;
		break;

	case eSelectionParam::EVFreshMilk:
	case eSelectionParam::EVFreshMilkDelay_dsec:
	case eSelectionParam::EVAirFreshMilk:
	case eSelectionParam::EVAirFreshMilkDelay_dsec:
	case eSelectionParam::CoffeWaterQty:
		if (paramValue > 500) 
			paramValue=500;
		break;

	case eSelectionParam::CoffeeQty:
		if (paramValue > 200) 
			paramValue=200;
		break;

	case eSelectionParam::FoamType:
		if (paramValue > 7) 
			paramValue=7;
		break;
	}

	u8 payload[4];
	payload[0] = selNum1ToN;
	payload[1] = paramID;
	rhea::utils::bufferWriteU16_LSB_MSB (&payload[2], paramValue);
	return buildMsg_Programming(eCPUProgrammingCommand::setSelectionParam, payload, 4, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_getSelectionParam (u8 selNum1ToN, eSelectionParam whichParam, u8* out_buffer, u32 sizeOfOutBuffer)
{
	if (selNum1ToN<1 || selNum1ToN > NUM_MAX_SELECTIONS)
	{
		DBGBREAK;
		return 0;
	}

	const u8 paramID = static_cast<u8>(whichParam);
	u8 payload[4];
	payload[0] = selNum1ToN;
	payload[1] = paramID;
	return buildMsg_Programming(eCPUProgrammingCommand::getSelectionParam, payload, 2, out_buffer, sizeOfOutBuffer);
}

//***************************************************
void cpubridge::subscribe(const HThreadMsgW &hCPUMsgQWrite, const HThreadMsgW &hOtherMsgQWrite, u16 applicationUID)
{
	const u32 param32 = hOtherMsgQWrite.asU32();
	u8 payload[2];
	rhea::utils::bufferWriteU16(payload, applicationUID);
	rhea::thread::pushMsg (hCPUMsgQWrite, CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST, param32, payload, 2);
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
	rhea::thread::pushMsg(sub.hFromSubscriberToMeW, CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST, (u32)0);
}


//***************************************************
void cpubridge::notify_CPUBRIDGE_DYING (const sSubscriber &to)
{
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_DYING, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_STATE_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, cpubridge::eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType, u16 flag1)
{
	u8 state[8];
	state[0] = (u8)VMCstate;
	state[1] = (u8)VMCerrorCode;
	state[2] = (u8)VMCerrorType;
	rhea::utils::bufferWriteU16(&state[3], flag1);

	logger->log("notify_CPU_STATE_CHANGED [%d] [%d] [%d]\n", VMCstate, VMCerrorCode, VMCerrorType);

	rhea::thread::pushMsg (to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED, handlerID, state, 5);
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
void cpubridge::notify_CPU_CUR_SEL_RUNNING(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum)
{
	logger->log("notify_CPU_CUR_SEL_RUNNING\n");

	u8 state[2];
	state[0] = selNum;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CUR_SEL_RUNNING, handlerID, state, 1);
}

//***************************************************
void cpubridge::translateNotify_CPU_CUR_SEL_RUNNING(const rhea::thread::sMsg &msg, u8 *out_selNum)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CUR_SEL_RUNNING);
	const u8 *data = (u8*)msg.buffer;
	*out_selNum = data[0];
}

//***************************************************
void cpubridge::notify_CPU_CREDIT_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const void *credit, u8 sizeOfCredit)
{
	logger->log("notify_CPU_CREDIT_CHANGED\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED, handlerID, credit, sizeOfCredit);
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
			//rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, handlerID, msg, sizeof(sCPULCDMessage));
			rhea::thread::pushMsg2Buffer(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, handlerID, &msg->importanceLevel, 1, msg, i*2);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED, handlerID, selAvailability->_flag, sizeof(selAvailability->_flag));
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

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED, handlerID, buffer, sizeof(u16) * (numPrices+1) );
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
void cpubridge::notify_CPU_GET_PRICEHOLDING_PRICELIST (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 firstPrice, u8 numPrices, const u16 *prices)
{
	logger->log("notify_CPU_GET_PRICEHOLDING_PRICELIST\n");
	u16 buffer[96];
	buffer[0] = (u16)firstPrice | ((u16)numPrices << 8);
	memcpy(&buffer[1], prices, sizeof(u16)* numPrices);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_PRICEHOLDING_PRICELIST, handlerID, buffer, sizeof(u16) * (numPrices + 1));
}

//***************************************************
void cpubridge::translateNotifyCPU_GET_PRICEHOLDING_PRICELIST (const rhea::thread::sMsg &msg, u8 *out_firstPrice, u8 *out_numPrices, u16 *out_prices)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_PRICEHOLDING_PRICELIST);
	const u16 *p = (const u16*)msg.buffer;
	*out_firstPrice = (u8)(p[0] & 0x00FF);
	*out_numPrices = (u8)((p[0] & 0xFF00) >> 8);
	memcpy(out_prices, &p[1], sizeof(u16)* (*out_numPrices));
}

//***************************************************
void cpubridge::notify_CPU_SINGLE_SEL_PRICE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum, const u8* utf8_alreadyFormattedPriceString)
{
    logger->log("notify_CPU_SINGLE_SEL_PRICE\n");

    const u32 len = rhea::string::utf8::lengthInBytes(utf8_alreadyFormattedPriceString);
    rhea::thread::pushMsg2Buffer(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SINGLE_SEL_PRICE_STRING, handlerID,
                                 &selNum, 1,
                                 utf8_alreadyFormattedPriceString, len+1);
}

//***************************************************
void cpubridge::translateNotify_CPU_SINGLE_SEL_PRICE(const rhea::thread::sMsg &msg, u8 *out_selNum, u8* out_utf8_alreadyFormattedPriceString, u32 sizeOfUtf8FormattedPriceString)
{
    assert(msg.what == CPUBRIDGE_NOTIFY_SINGLE_SEL_PRICE_STRING);
    const u8 *pp = (const u8*)msg.buffer;

    *out_selNum = pp[0];

    const u8 *priceStr = &pp[1];
    rhea::string::utf8::copyStrAsMuchAsYouCan (out_utf8_alreadyFormattedPriceString, sizeOfUtf8FormattedPriceString, priceStr);
}

//***************************************************
void cpubridge::notify_CPU_RUNNING_SEL_STATUS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eRunningSelStatus status)
{
	logger->log("notify_CPU_RUNNING_SEL_STATUS [%d]\n", (u8)status);

	u32 paramU32 = (u32)status;
	paramU32 <<= 16;
	paramU32 |= handlerID;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS, paramU32);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_FULLSTATE, handlerID, s, sizeof(sCPUStatus));
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_INI_PARAM, handlerID, s, sizeof(sCPUParamIniziali));
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED, handlerID);
}

//***************************************************
void cpubridge::translateNotify_CPU_BTN_PROG_PRESSED(const rhea::thread::sMsg &msg)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED);
}

//***************************************************
void cpubridge::notify_READ_DATA_AUDIT_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID, const void *readData, u8 nBytesInReadData)
{
	if (NULL != readData)
		logger->log("notify_READ_DATA_AUDIT_PROGRESS [st:%d] [%d bytes]\n", status, nBytesInReadData);
	else
		logger->log("notify_READ_DATA_AUDIT_PROGRESS [st:%d]\n", status);

	u8 buffer[512];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU16(fileID);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);

	if (NULL != readData && nBytesInReadData>0)
		nbw.writeBlob (readData, nBytesInReadData);

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_READ_DATA_AUDIT_PROGRESS (const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID, u8 *out_readData, u8 *out_nBytesInReadData)
{
    assert(msg.what == CPUBRIDGE_NOTIFY_READ_DATA_AUDIT_PROGRESS);
	assert (	(NULL == out_readData) ||
				(NULL != out_readData && NULL != out_nBytesInReadData)
			);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eEndianess::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_fileID = u;
	nbr.readU16(u); *out_totKbSoFar = u;
	
    u8 b = 0;
	nbr.readU8(b); *out_status = (eReadDataFileStatus)b;

	if (NULL != out_nBytesInReadData)
	{
		*out_nBytesInReadData = 0;
		if (msg.bufferSize > 5)
		{
			const u8 readDataLen = msg.bufferSize - 5;
			*out_nBytesInReadData = readDataLen;
			nbr.readBlob (out_readData, readDataLen);
		}
	}
}

//***************************************************
void cpubridge::notify_READ_VMCDATAFILE_PROGRESS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eReadDataFileStatus status, u16 totKbSoFar, u16 fileID)
{
	logger->log("notify_READ_VMCDATAFILE_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU16(fileID);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_READ_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eReadDataFileStatus *out_status, u16 *out_totKbSoFar, u16 *out_fileID)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_READ_VMCDATAFILE_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eEndianess::eBigEndian);

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
	nbw.setup(buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU16(totKbSoFar);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_WRITE_VMCDATAFILE_PROGRESS(const rhea::thread::sMsg &msg, eWriteDataFileStatus *out_status, u16 *out_totKbSoFar)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_VMCDATAFILE_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eEndianess::eBigEndian);

    u16 u = 0;
	nbr.readU16(u); *out_totKbSoFar = u;

    u8 b = 0;
	nbr.readU8(b); *out_status = (eWriteDataFileStatus)b;
}

//***************************************************
void cpubridge::notify_WRITE_CPUFW_PROGRESS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eWriteCPUFWFileStatus status, u16 param)
{
	logger->log("notify_WRITE_CPUFW_PROGRESS\n");

	u8 buffer[8];
	rhea::NetStaticBufferViewW nbw;
	nbw.setup(buffer, sizeof(buffer), rhea::eEndianess::eBigEndian);
	nbw.writeU16(param);
	nbw.writeU8((u8)status);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS, handlerID, buffer, nbw.length());
}

//***************************************************
void cpubridge::translateNotify_WRITE_CPUFW_PROGRESS(const rhea::thread::sMsg &msg, eWriteCPUFWFileStatus *out_status, u16 *out_param)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_CPUFW_PROGRESS);

	rhea::NetStaticBufferViewR nbr;
	nbr.setup(msg.buffer, msg.bufferSize, rhea::eEndianess::eBigEndian);

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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP, handlerID, buffer, ts.getLenInBytes());
}

//***************************************************
void cpubridge::translateNotify_CPU_VMCDATAFILE_TIMESTAMP(const rhea::thread::sMsg &msg, sCPUVMCDataFileTimeStamp *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_VMCDATAFILE_TIMESTAMP);

	out->readFromBuffer(msg.buffer);
}


//***************************************************
void cpubridge::notify_SAN_WASHING_STATUS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 b0, u8 b1, u8 b2, const u8 *buffer8)
{
	logger->log("notify_SAN_WASHING_STATUS\n");
	u8 buffer[16];
	buffer[0] = b0;
	buffer[1] = b1;
	buffer[2] = b2;
	memcpy (&buffer[3], buffer8, 8);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS, handlerID, buffer, 11);
}

//***************************************************
void cpubridge::translateNotify_SAN_WASHING_STATUS(const rhea::thread::sMsg &msg, u8 *out_b0, u8 *out_b1, u8 *out_b2, u8 *out_bufferDiAlmeno8Byte)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SANWASH_STATUS);

	const u8 *p = (const u8*)msg.buffer;
	*out_b0 = p[0];
	*out_b1 = p[1];
	*out_b2 = p[2];
	memcpy (out_bufferDiAlmeno8Byte, &p[3], 8);
}


//***************************************************
void cpubridge::notify_WRITE_PARTIAL_VMCDATAFILE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 blockNumOffset)
{
	logger->log("notify_WRITE_PARTIAL_VMCDATAFILE [%d]\n", blockNumOffset);
	u8 buffer[4] = { blockNumOffset, 0, 0, 0 };
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_WRITE_PARTIAL_VMCDATAFILE_PROGRESS, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_WRITE_PARTIAL_VMCDATAFILE(const rhea::thread::sMsg &msg, u8 *out_blockNumOffset)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_WRITE_PARTIAL_VMCDATAFILE_PROGRESS);

	const u8 *p = (const u8*)msg.buffer;
	*out_blockNumOffset = p[0];
}

//***************************************************
void cpubridge::notify_CPU_DECOUNTER_SET(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProg_decounter which, u16 valore)
{
	logger->log("notify_CPU_DECOUNTER_SET [%d] [%d]\n", (u8)which, valore);
	u8 buffer[4];
	buffer[0] = (u8)which;
	rhea::utils::bufferWriteU16 (&buffer[1], valore);
	rhea::thread::pushMsg (to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_CPU_DECOUNTER_SET(const rhea::thread::sMsg &msg, eCPUProg_decounter *out_which, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_DECOUNTER_SET);

	const u8 *p = (const u8*)msg.buffer;
	*out_which = (eCPUProg_decounter)p[0];
	*out_valore = rhea::utils::bufferReadU16 (&p[1]);
}

//***************************************************
void cpubridge::notify_CPU_ALL_DECOUNTER_VALUES(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u32 *arrayDiAlmeno15Elementi, u32 sizeof_in_array)
{
	assert (sizeof_in_array >= 15 * sizeof(u32));
	logger->log("notify_CPU_ALL_DECOUNTER_VALUES\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_ALL_DECOUNTER_VALUES, handlerID, arrayDiAlmeno15Elementi, 15 * sizeof(u32));
}

//***************************************************
void cpubridge::translateNotify_CPU_ALL_DECOUNTER_VALUES(const rhea::thread::sMsg &msg, u32 *out_arrayDiAlmeno15Elementi, u32 sizeof_out_array)
{
	assert (sizeof_out_array >= 15 * sizeof(u32));
	assert(msg.what == CPUBRIDGE_NOTIFY_ALL_DECOUNTER_VALUES);
	memcpy(out_arrayDiAlmeno15Elementi, msg.buffer, sizeof(u32) * 15);
}

//***************************************************
void cpubridge::notify_EXTENDED_CONFIG_INFO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sExtendedCPUInfo *info)
{
	logger->log("notify_EXTENDED_CONFIG_INFO\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_EXTENDED_CONFIG_INFO, handlerID, info, sizeof(sExtendedCPUInfo));
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
	logger->log("notify_ATTIVAZIONE_MOTORE m=%d, durata=%d, numRep=%d\n",motore_1_10, durata_dSec, numRipetizioni);

	u16 buffer[4];
	buffer[0] = motore_1_10;
	buffer[1] = durata_dSec;
	buffer[2] = numRipetizioni;
	buffer[3] = pausaTraRipetizioni_dSec;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_ATTIVAZIONE_MOTORE, handlerID, buffer, 4);
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
void cpubridge::notify_CALCOLA_IMPULSI_GRUPPO_STARTED(const sSubscriber &to, u16 handlerID, u8 macina_1to4, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CALCOLA_IMPULSI_GRUPPO_STARTED, m=%d\n", macina_1to4);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CALCOLA_IMPULSI_GRUPPO_STARTED, handlerID);
}

//***************************************************
void cpubridge::notify_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 stato, u16 valore)
{
	logger->log("notify_STATO_CALCOLO_IMPULSI_GRUPPO\n");

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, valore);
	buffer[2] = stato;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_STATO_CALCOLO_IMPULSI_GRUPPO, handlerID, buffer, 3);
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
void cpubridge::notify_SET_FATTORE_CALIB_MOTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProg_motor motore, u16 valore)
{
	logger->log("notify_SET_FATTORE_CALIB_MOTORE m=%d, v=%d\n", motore , valore);

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, valore);
	buffer[2] = (u8)motore;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SET_FATTORE_CALIB_MOTORE, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProg_motor *out_motore, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SET_FATTORE_CALIB_MOTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_valore = rhea::utils::bufferReadU16(p);
	*out_motore = (eCPUProg_motor)p[2];
}

//***************************************************
void cpubridge::notify_STATO_GRUPPO(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUProg_statoGruppo stato)
{
	logger->log("notify_STATO_GRUPPO\n");

	u8 buffer[4];
	buffer[0] = (u8)stato;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_STATO_GRUPPO, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_STATO_GRUPPO(const rhea::thread::sMsg &msg, eCPUProg_statoGruppo *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_STATO_GRUPPO);
	const u8 *p = (const u8*)msg.buffer;
	*out = (eCPUProg_statoGruppo)p[0];
}

//***************************************************
void cpubridge::notify_GET_TIME(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm, u8 ss)
{
	logger->log("notify_GET_TIME\n");

	u8 buffer[4];
	buffer[0] = hh;
	buffer[1] = mm;
	buffer[2] = ss;
	
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_TIME, handlerID, buffer, 3);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_DATE, handlerID, buffer, 4);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SET_TIME, handlerID, buffer, 3);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SET_DATE, handlerID, buffer, 4);
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
	logger->log("notify_CPU_POSIZIONE_MACINA %d\n", macina_1o2);

	u8 buffer[4];
	rhea::utils::bufferWriteU16(buffer, posizione);
	buffer[2] = macina_1o2;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_POSIZIONE_MACINA, handlerID, buffer, 3);
}

//***************************************************
void cpubridge::translateNotify_CPU_POSIZIONE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1o2, u16 *out_posizione)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_posizione = rhea::utils::bufferReadU16(p);
	*out_macina_1o2 = p[2];
}

//***************************************************
void cpubridge::notify_CPU_MOTORE_MACINA(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 macina_1to4, eCPUProg_macinaMove m)
{
	logger->log("notify_CPU_MOTORE_MACINA %d\n", macina_1to4);

	u8 buffer[4];
	buffer[0] = macina_1to4;
	buffer[1] = (u8)m;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_MOTORE_MACINA, handlerID, buffer, 2);
}

//***************************************************
void cpubridge::translateNotify_CPU_MOTORE_MACINA(const rhea::thread::sMsg &msg, u8 *out_macina_1to4, eCPUProg_macinaMove *out_m)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_MOTORE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1to4 = p[0];
	*out_m = (eCPUProg_macinaMove)p[1];
}

//***************************************************
void cpubridge::notify_CPU_TEST_SELECTION(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum, eCPUProg_testSelectionDevice d)
{
	logger->log("notify_CPU_TEST_SELECTION\n");

	u8 buffer[4];
	buffer[0] = selNum;
	buffer[1] = (u8)d;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_TEST_SELECTION, handlerID, buffer, 2);
}
//***************************************************
void cpubridge::translateNotify_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProg_testSelectionDevice *out_d)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_TEST_SELECTION);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_d = (eCPUProg_testSelectionDevice)p[1];
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_NOMI_LINGUE_CPU, handlerID, buffer, sizeof(buffer));
}

//***************************************************
void cpubridge::translateNotify_NOMI_LINGE_CPU(const rhea::thread::sMsg &msg, u16 *out_strLingua1UTF16, u16 *out_strLingua2UTF16)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_NOMI_LINGUE_CPU);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_EVA_RESET_PARTIALDATA, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_EVA_RESET_PARTIALDATA(const rhea::thread::sMsg &msg, bool *out_result)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_EVA_RESET_PARTIALDATA);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_VOLT_AND_TEMP, handlerID, buffer, 5);
}

//***************************************************
void cpubridge::translateNotify_GET_VOLT_AND_TEMP(const rhea::thread::sMsg &msg, u8 *out_tCamera, u8 *out_tBollitore, u8 *out_tCappuccinatore, u16 *out_voltaggio)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_VOLT_AND_TEMP);
	const u8 *p = (const u8*)msg.buffer;
	*out_tCamera = p[0];
	*out_tBollitore = p[1];
	*out_tCappuccinatore = p[2];
	*out_voltaggio = rhea::utils::bufferReadU16(&p[3]);
}

//***************************************************
void cpubridge::notify_GET_OFF_REPORT(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 indexNum, u8 lastIndexNum, const sCPUOffSingleEvent *offs, u8 numOffs)
{
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

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_OFF_REPORT, handlerID, buffer, ct);
}

//***************************************************
void cpubridge::translateNotify_GET_OFF_REPORT(const rhea::thread::sMsg &msg, u8 *out_indexNum, u8 *out_lastIndexNum, u8 *out_numOffs, sCPUOffSingleEvent *out, u32 sizeofOut)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_OFF_REPORT);
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

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_LAST_FLUX_INFORMATION, handlerID, buffer, 4);
}

//***************************************************
void cpubridge::translateNotify_GET_LAST_FLUX_INFORMATION(const rhea::thread::sMsg &msg, u16 *out_lastFlux, u16 *out_lastGrinderPosition)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_LAST_FLUX_INFORMATION);
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
		rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_CPU_STRING_MODEL_AND_VER, handlerID, utf16_msg, (n+1)*2);
}

//***************************************************
void cpubridge::translateNotify_CPU_STRING_VERSION_AND_MODEL(const rhea::thread::sMsg &msg, u16 *out_utf16msg, u32 sizeOfOutUTF16MsgInBytes)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_CPU_STRING_MODEL_AND_VER);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_START_MODEM_TEST, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_CPU_EVA_RESET_TOTALS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_EVA_RESET_TOTALS\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_EVA_RESET_TOTALS, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 hh, u8 mm)
{
	logger->log("notify_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE\n");

	u8 buffer[4];
	buffer[0] = hh;
	buffer[1] = mm;

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_TIME_LAVSAN_CAPPUCINATORE, handlerID, buffer, 2);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_START_TEST_ASSORBIMENTO_MOTORIDUTTORE\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_START_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
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

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GETSTATUS_TEST_ASSORBIMENTO_GRUPPO, handlerID, buffer, 26);
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

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GETSTATUSTEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, buffer, 6);
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
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_MILKER_VER, handlerID, ver, len+1);
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
void cpubridge::notify_CPU_START_GRINDER_SPEED_TEST (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool bStarted)
{
	logger->log("notify_CPU_START_GRINDER_SPEED_TEST\n");

	u8 data = 0;
	if (bStarted)
		data = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_START_GRINDER_SPEED_TEST, handlerID, &data, 1);
}

//***************************************************
void cpubridge::translateNotify_CPU_START_GRINDER_SPEED_TEST (const rhea::thread::sMsg &msg, bool *out_bStarted)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_START_GRINDER_SPEED_TEST);

	*out_bStarted = false;
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bStarted = true;
}

//***************************************************
void cpubridge::notify_CPU_GET_LAST_GRINDER_SPEED (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 speed)
{
	logger->log("notify_CPU_GET_LAST_GRINDER_SPEED\n");
	u8 data[2];
	rhea::utils::bufferWriteU16(data, speed);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_LAST_GRINDER_SPEED, handlerID, data, 2);
}

//***************************************************
void cpubridge::translateNotify_CPU_GET_LAST_GRINDER_SPEED (const rhea::thread::sMsg &msg, u16 *out_speed)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_LAST_GRINDER_SPEED);

	const u8 *p = (const u8*)msg.buffer;
	*out_speed = rhea::utils::bufferReadU16(p);
}

//***************************************************
void cpubridge::notify_CPU_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum, const u16 *utf16_LSB_MSB_selectioName)
{
	logger->log("notify_CPU_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB\n");

	const u8 len = (u8)rhea::string::utf16::lengthInBytes(utf16_LSB_MSB_selectioName);
	
	u8 data[2];
	data[0] = selNum;
	data[1] = len;
	rhea::thread::pushMsg2Buffer(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SELECTION_NAME_UTF16_LSB_MSB, handlerID, data, 2, utf16_LSB_MSB_selectioName, len);
}
void cpubridge::translateNotify_CPU_GET_CPU_SELECTION_NAME (const rhea::thread::sMsg &msg, u8 *out_selNum, u16 *out_utf16_LSB_MSB_selectioName, u32 sizeOfOutUTF16_LSB_MSB_selectioName)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SELECTION_NAME_UTF16_LSB_MSB);

	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	u8 len = p[1];

	memset (out_utf16_LSB_MSB_selectioName, 0, sizeOfOutUTF16_LSB_MSB_selectioName);
    if (sizeOfOutUTF16_LSB_MSB_selectioName < (u32)(len+2))
		len = sizeOfOutUTF16_LSB_MSB_selectioName - 2;
	
	if (len)
		memcpy (out_utf16_LSB_MSB_selectioName, &p[2], len);
}

//***************************************************
void cpubridge::notify_MSG_FROM_LANGUAGE_TABLE (const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, u8 tableID, u8 msgRowNum, const u8 *utf8message)
{
	logger->log("notify_MSG_FROM_LANGUAGE_TABLE [tabID=%d] [msgRow=%d]\n", tableID, msgRowNum);

	const u8 len = (u8)rhea::string::utf8::lengthInBytes(utf8message);
	
	u8 data[4];
	data[0] = tableID;
	data[1] = msgRowNum;
	data[2] = len;
	rhea::thread::pushMsg2Buffer(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_MSG_FROM_LANGUAGE_TABLE, handlerID, data, 3, utf8message, len);
}
void cpubridge::translateNotify_MSG_FROM_LANGUAGE_TABLE (const rhea::thread::sMsg &msg, u8 *out_tableID, u8 *out_msgRowNum, u8 *out_utf8message, u32 sizeOf_utf8message)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_MSG_FROM_LANGUAGE_TABLE);

	const u8 *p = (const u8*)msg.buffer;
	*out_tableID = p[0];
	*out_msgRowNum = p[1];
	u8 len = p[2];

	memset (out_utf8message, 0, sizeOf_utf8message);
    if (sizeOf_utf8message < (u32)(len+2))
		len = sizeOf_utf8message - 2;
	
	if (len)
		memcpy (out_utf8message, &p[3], len);
}






//***************************************************
void cpubridge::ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION, (u32)selNumber);
}
void cpubridge::translate_CPU_START_SELECTION (const rhea::thread::sMsg &msg, u8 *out_selNumber)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION);
    *out_selNumber = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::ask_CPU_START_SELECTION_AND_FORCE_JUG (const sSubscriber &from, u8 selNumber)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION_AND_FORCE_JUG, (u32)selNumber);
}
void cpubridge::translate_CPU_START_SELECTION_AND_FORCE_JUG(const rhea::thread::sMsg &msg, u8 *out_selNumber)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION_AND_FORCE_JUG);
    *out_selNumber = (u8)msg.paramU32;
}



//***************************************************
void cpubridge::ask_CPU_STOP_SELECTION (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION, (u32)0);
}

//***************************************************
void cpubridge::ask_CPU_SEND_BUTTON(const sSubscriber &from, u8 buttonNum)
{
    rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_SEND_BUTTON_NUM, (u32)buttonNum);
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
    rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_KEEP_SENDING_BUTTON_NUM, (u32)buttonNum);
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
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_FULLSTATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_INI_PARAM(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_AVAIL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_PRICES (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SINGLE_SEL_PRICE (const sSubscriber &from, u16 handlerID, u8 selNum)
{
    rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_SINGLE_SEL_PRICE, handlerID, &selNum, 1);
}

//***************************************************
void cpubridge::translate_CPU_QUERY_SINGLE_SEL_PRICE(const rhea::thread::sMsg &msg, u8 *out_selNum)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_SINGLE_SEL_PRICE);
    const u8* p = (const u8*)msg.buffer;
    *out_selNum = p[0];
}

//***************************************************
void cpubridge::ask_CPU_QUERY_ID101 (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_ID_101, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_RESTART (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_RESTART, handlerID);
}
void cpubridge::notify_CPU_RESTART  (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_RESTART\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_RESTART, handlerID);
}

//***************************************************
void cpubridge::ask_SET_MACHINE_LOCK_STATUS (const sSubscriber &from, u16 handlerID, eLockStatus lockStatus)
{
	u8 buffer[2];
	buffer[0] = static_cast<u8>(lockStatus);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_MACHINE_LOCK_STATUS, handlerID, buffer, 1);
}
void cpubridge::translate_SET_MACHINE_LOCK_STATUS(const rhea::thread::sMsg &msg, eLockStatus *out_lockStatus)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_MACHINE_LOCK_STATUS);
    const u8* p = (const u8*)msg.buffer;
    *out_lockStatus = static_cast<eLockStatus>(p[0]);
}

//***************************************************
void cpubridge::ask_GET_MACHINE_LOCK_STATUS (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_MACHINE_LOCK_STATUS, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_LCD_MESSAGE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_CURRENT_CREDIT(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_STATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_CUR_SEL_RUNNING(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_QUERY_CUR_SEL_RUNNING, handlerID);
}

//***************************************************
void cpubridge::ask_READ_DATA_AUDIT(const sSubscriber &from, u16 handlerID, bool bIncludeDataInNotify)
{
	u8 buffer[2];
	buffer[0] = 0;
	if (bIncludeDataInNotify)
		buffer[0] = 0x01;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT, handlerID, buffer, 1);
}
void cpubridge::translate_READ_DATA_AUDIT(const rhea::thread::sMsg &msg, bool *out_bIncludeDataInNotify)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_READ_DATA_AUDIT);
	
	*out_bIncludeDataInNotify = false;
	u32 n = msg.bufferSize;
	if (n > 0)
	{
		const u8 *p = (const u8*)msg.buffer;
		if (p[0] == 0x01)
			*out_bIncludeDataInNotify = true;
	}
}


//***************************************************
void cpubridge::ask_READ_VMCDATAFILE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_READ_VMCDATAFILE, handlerID);
}

//***************************************************
void cpubridge::ask_WRITE_VMCDATAFILE(const sSubscriber &from, u16 handlerID, const u8* const srcFullFileNameAndPath)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_VMCDATAFILE, handlerID, srcFullFileNameAndPath, rhea::string::utf8::lengthInBytes(srcFullFileNameAndPath)+1);
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
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_PARTIAL_VMCDATAFILE, handlerID, buffer, sizeof(buffer));
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
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_VMCDATAFILE_TIMESTAMP, handlerID);
}


//***************************************************
void cpubridge::ask_WRITE_CPUFW(const sSubscriber &from, u16 handlerID, const u8* const srcFullFileNameAndPath)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_WRITE_CPUFW, handlerID, srcFullFileNameAndPath, rhea::string::utf8::lengthInBytes(srcFullFileNameAndPath) + 1);
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
    rhea::thread::pushMsg (from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_PROGRAMMING_CMD, handlerID, otherData, 1+ sizeOfOptionalData);
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
void cpubridge::ask_CPU_SET_DECOUNTER (const sSubscriber &from, u16 handlerID, eCPUProg_decounter which, u16 valore)
{
	u8 otherData[4];
	otherData[0] = (u8)which;
	rhea::utils::bufferWriteU16 (&otherData[1], valore);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_DECOUNTER (const rhea::thread::sMsg &msg, eCPUProg_decounter *out_which, u16 *out_valore)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_DECOUNTER);
	const u8 *p = (const u8*)msg.buffer;
	*out_which = (eCPUProg_decounter)p[0];
	*out_valore = rhea::utils::bufferReadU16 (&p[1]);
}

//***************************************************
void cpubridge::ask_CPU_GET_ALL_DECOUNTER_VALUES(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_ALL_DECOUNTER_VALUES, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_EXTENDED_CONFIG_INFO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_EXTENDED_CONFIG_INFO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_MILKER_TYPE (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_GET_MILKER_TYPE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_ATTIVAZIONE_MOTORE(const sSubscriber &from, u16 handlerID, u8 motore_1_10, u8 durata_dSec, u8 numRipetizioni, u8 pausaTraRipetizioni_dSec)
{
	u8 otherData[4];
	otherData[0] = motore_1_10;
	otherData[1] = durata_dSec;
	otherData[2] = numRipetizioni;
	otherData[3] = pausaTraRipetizioni_dSec;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_ATTIVAZIONE_MOTORE, handlerID, otherData, 4);
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
void cpubridge::ask_CPU_CALCOLA_IMPULSI_GRUPPO_AA (const sSubscriber &from, u16 handlerID, u8 macina_1to4, u16 totalePesata_dGrammi)
{
	if (macina_1to4 < 1 || macina_1to4>4)
	{
		DBGBREAK;
		macina_1to4 = 1;
	}

	u8 otherData[4];
	otherData[0] = macina_1to4;
	rhea::utils::bufferWriteU16(&otherData[1], totalePesata_dGrammi);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_CALCOLA_IMPULSI_GRUPPO_AA (const rhea::thread::sMsg &msg, u8 *out_macina_1to4, u16 *out_totalePesata_dGrammi)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CALCOLA_IMPULSI_GRUPPO);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1to4 = p[0];
	*out_totalePesata_dGrammi = rhea::utils::bufferReadU16(&p[1]);
}

//***************************************************
void cpubridge::ask_CPU_START_GRINDER_SPEED_TEST_AA(const sSubscriber &from, u16 handlerID, u8 macina_1to4, u8 durataMacinataInSec)
{
	u8 otherData[2];
	otherData[0] = macina_1to4;
	otherData[1] = durataMacinataInSec;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_START_GRINDER_SPEED_TEST, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_START_GRINDER_SPEED_TEST_AA (const rhea::thread::sMsg &msg, u8 *out_macina_1to4, u8 *out_durataMacinataInSec)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_START_GRINDER_SPEED_TEST);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1to4 = p[0];
	*out_durataMacinataInSec = p[1];
}

//***************************************************
void cpubridge::ask_CPU_GET_LAST_GRINDER_SPEED (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_GRINDER_SPEED, handlerID);
}


//***************************************************
void cpubridge::ask_CPU_GET_STATO_CALCOLO_IMPULSI_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_CALCOLO_IMPULSI_GRUPPO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_SET_FATTORE_CALIB_MOTORE(const sSubscriber &from, u16 handlerID, eCPUProg_motor motore, u16 valoreGr)
{
	u8 otherData[4];
	otherData[0] = (u8)motore;
	rhea::utils::bufferWriteU16(&otherData[1], valoreGr);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_FATTORE_CALIB_MOTORE(const rhea::thread::sMsg &msg, eCPUProg_motor *out_motore, u16 *out_valoreGr)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_FATTORE_CALIB_MOTORE);
	const u8 *p = (const u8*)msg.buffer;
	*out_motore = (eCPUProg_motor)p[0];
	*out_valoreGr = rhea::utils::bufferReadU16(&p[1]);
}

//***************************************************
void cpubridge::ask_CPU_GET_PRICEHOLDING_PRICELIST (const sSubscriber &from, u16 handlerID, u8 firstPrice, u8 numPrices)
{
	u8 otherData[2];
	otherData[0] = firstPrice;
	otherData[1] = numPrices;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_GET_PRICEHOLDING_PRICELIST, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_GET_PRICEHOLDING_PRICELIST(const rhea::thread::sMsg &msg, u8 *out_firstPrice, u8 *out_numPrices)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_GET_PRICEHOLDING_PRICELIST);
	const u8 *p = (const u8*)msg.buffer;
	*out_firstPrice = p[0];
	*out_numPrices = p[1];
}



//***************************************************
void cpubridge::ask_CPU_GET_STATO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_STATO_GRUPPO, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_TIME(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_TIME, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_DATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_DATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_VOLT_AND_TEMP(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_VOLT_AND_TEMP, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_OFF_REPORT(const sSubscriber &from, u16 handlerID, u8 indexNum)
{
	u8 otherData[4];
	otherData[0] = indexNum;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_OFF_REPORT, handlerID, otherData, 1);
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
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_LAST_FLUX_INFORMATION, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_SHOW_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SHOW_STR_VERSION_AND_MODEL, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_STRING_VERSION_AND_MODEL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_STR_VERSION_AND_MODEL, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_DA3SYNC(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_DA3_SYNC, 0, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_START_MODEM_TEST(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_START_MODEM_TEST, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_SET_TIME(const sSubscriber &from, u16 handlerID, u8 hh, u8 mm, u8 ss)
{
	u8 otherData[4];
	otherData[0] = hh;
	otherData[1] = mm;
	otherData[2] = ss;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_TIME, handlerID, otherData, 3);
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
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_DATE, handlerID, otherData, 3);
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
void cpubridge::ask_CPU_GET_POSIZIONE_MACINA_AA(const sSubscriber &from, u16 handlerID, u8 macina_1to4)
{
	if (macina_1to4 < 1 || macina_1to4 > 4)
		macina_1to4 = 1;

	u8 otherData[1];
	otherData[0] = macina_1to4;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA, handlerID, otherData, 1);
}

//***************************************************
void cpubridge::translate_CPU_GET_POSIZIONE_MACINA_AA(const rhea::thread::sMsg &msg, u8 *out_macina_1to4)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_GET_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1to4 = p[0];
}

//***************************************************
void cpubridge::ask_CPU_SET_MOTORE_MACINA_AA (const sSubscriber &from, u16 handlerID, u8 macina_1to4, eCPUProg_macinaMove m)
{
	if (macina_1to4<1 || macina_1to4>4)
		macina_1to4 = 1;

	u8 otherData[2];
	otherData[0] = macina_1to4;
	otherData[1] = (u8)m;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_SET_MOTORE_MACINA_AA (const rhea::thread::sMsg &msg, u8 *out_macina_1to4, eCPUProg_macinaMove *out_m)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_MOTORE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macina_1to4 = p[0];
	*out_m = (eCPUProg_macinaMove)p[1];
}

//***************************************************
void cpubridge::ask_CPU_SET_POSIZIONE_MACINA_AA (const sSubscriber &from, u16 handlerID, u8 macina_1to4, u16 target)
{
	if (macina_1to4<1 || macina_1to4>4)
		macina_1to4=1;

	u8 otherData[4];
	rhea::utils::bufferWriteU16(otherData, target);
	otherData[2] = macina_1to4;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA, handlerID, otherData, 3);
}

//***************************************************
void cpubridge::translate_CPU_SET_POSIZIONE_MACINA_AA(const rhea::thread::sMsg &msg, u8 *out_macina_1to4, u16 *out_target)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_POSIZIONE_MACINA);
	const u8 *p = (const u8*)msg.buffer;
	*out_target = rhea::utils::bufferReadU16(p);
	*out_macina_1to4 = p[2];
}

//***************************************************
void cpubridge::ask_CPU_TEST_SELECTION(const sSubscriber &from, u16 handlerID, u8 selNum, eCPUProg_testSelectionDevice d)
{
	u8 otherData[4];
	otherData[0] = selNum;
	otherData[1] = (u8)d;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE, handlerID, otherData, 2);
}

//***************************************************
void cpubridge::translate_CPU_TEST_SELECTION(const rhea::thread::sMsg &msg, u8 *out_selNum, eCPUProg_testSelectionDevice *out_d)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_TEST_SELEZIONE);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_d = (eCPUProg_testSelectionDevice)p[1];
}

//***************************************************
void cpubridge::ask_CPU_GET_NOMI_LINGE_CPU(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_NOMI_LINGUE_CPU, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_DISINTALLAZIONE(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_DISINSTALLAZIONE, 0, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_RICARICA_FASCIA_ORARIA_FREEVEND(const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_RICARICA_FASCIA_ORARIA_FV, 0, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_EVA_RESET_PARTIALDATA(const sSubscriber &from, u16 handlerID UNUSED_PARAM)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_PARTIALDATA, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_EVA_RESET_TOTALS(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_EVA_RESET_TOTALS, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_GET_TIME_NEXT_LAVSAN_CAPPUCCINATORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_TIME_NEXT_LAVSAN_CAPPUCC, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_START_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_START_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_GRUPPO(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_GRUPPO, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_PROGRAMMING_CMD_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_QUERY_TEST_ASSORBIMENTO_MOTORIDUTTORE, handlerID, NULL, 0);
}

//***************************************************
void cpubridge::ask_CPU_MILKER_VER(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_MILKER_VER, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED (const sSubscriber &from, u8 selNum, u16 price, eGPUPaymentType paymentType, bool bForceJUG)
{
	u8 otherData[8];
	otherData[0] = selNum;
	otherData[1] = (u8)paymentType;
	rhea::utils::bufferWriteU16(&otherData[2], price);	
	if (bForceJUG)
		otherData[4] = 0x01;
	else
		otherData[4] = 0x00;

	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED, (u32)0, otherData, 5);
}

//***************************************************
void cpubridge::translate_CPU_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED(const rhea::thread::sMsg &msg, u8 *out_selNum, u16 *out_price, eGPUPaymentType *out_paymentType, bool *out_bForceJUG)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_START_SELECTION_WITH_PAYMENT_ALREADY_HANDLED);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
	*out_paymentType = (eGPUPaymentType)p[1];
	*out_price = rhea::utils::bufferReadU16 (&p[2]);
	*out_bForceJUG = false;
	if (p[4] != 0x00)
		*out_bForceJUG = true;
}

//***************************************************
void cpubridge::ask_CPU_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB (const sSubscriber &from, u16 handlerID, u8 selNum)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB, handlerID, &selNum, 1);
}

//***************************************************
void cpubridge::translate_CPU_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB (const rhea::thread::sMsg &msg, u8 *out_selNum)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_GET_CPU_SELECTION_NAME_UTF16_LSB_MSB);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum = p[0];
}

//***************************************************
void cpubridge::ask_CPU_VALIDATE_QUICK_MENU_PINCODE (const sSubscriber &from, u16 handlerID, u16 pinCode)
{
	u8 otherData[2];
	rhea::utils::bufferWriteU16 (otherData, pinCode);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_VALIDATE_QUICK_MENU_PINCODE, handlerID, otherData, 2);
}
void cpubridge::translate_CPU_VALIDATE_QUICK_MENU_PINCODE (const rhea::thread::sMsg &msg, u16 *out_pinCode)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_VALIDATE_QUICK_MENU_PINCODE);
	const u8 *p = (const u8*)msg.buffer;
	*out_pinCode = rhea::utils::bufferReadU16(p);
}


//***************************************************
void cpubridge::ask_END_OF_GRINDER_CLEANING_PROCEDURE (const sSubscriber &from, u16 handlerID, u8 grinder1toN)
{
	u8 otherData[2];
    otherData[0] = grinder1toN;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_END_OF_GRINDER_CLEANING_PROC, handlerID, otherData, 1);
}
void cpubridge::translate_END_OF_GRINDER_CLEANING_PROCEDURE (const rhea::thread::sMsg &msg, u8 *out_grinder1toN)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_END_OF_GRINDER_CLEANING_PROC);
	const u8 *p = (const u8*)msg.buffer;
    *out_grinder1toN = p[0];
}

//***************************************************
void cpubridge::ask_CPU_BROWSER_URL_CHANGE (const sSubscriber &from, u16 handlerID, const char *url)
{
	u8 otherData[512];
	memset (otherData, 0, sizeof(otherData));

	u32 len = rhea::string::utf8::lengthInBytes (reinterpret_cast<const u8*>(url));
	if (len > 200)
		len = 200;

	otherData[0] = static_cast<u8>(len);
	if (len)
		memcpy (&otherData[1], url, len);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_BROWSER_URL_CHANGE, handlerID, otherData, len+1);
}
void cpubridge::translate_CPU_BROWSER_URL_CHANGE (const rhea::thread::sMsg &msg, char *out_url, u32 sizeof_out_url)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_BROWSER_URL_CHANGE);
	const u8 *p = (const u8*)msg.buffer;
	
	u8 len = p[0];
	if (len >= sizeof_out_url)
		len = sizeof_out_url-1;
	if (len)
		memcpy (out_url, &p[1], len);
	out_url[len] = 0x00;
}

//***************************************************
void cpubridge::ask_CPU_ATTIVAZIONE_SCIVOLO_BREWMATIC (const sSubscriber &from, u16 handlerID, u8 perc0_100)
{
	u8 otherData[2];
	otherData[0] = perc0_100;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SCIVOLO_BREWMATIC, handlerID, otherData, 1);
}
void cpubridge::translate_CPU_ATTIVAZIONE_SCIVOLO_BREWMATIC (const rhea::thread::sMsg &msg, u8 *out_perc0_100)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SCIVOLO_BREWMATIC);
	const u8 *p = (const u8*)msg.buffer;
	*out_perc0_100 = p[0];
}

void cpubridge::ask_MSG_FROM_LANGUAGE_TABLE (const sSubscriber &from, u16 handlerID, u8 tableID, u8 msgRowNum, u8 language1or2)
{
	assert (language1or2==1 || language1or2==2);

	u8 payload[4];
	payload[0] = tableID;
	payload[1] = msgRowNum;
	payload[2] = language1or2;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_MSG_FROM_LANGUAGE_TABLE, handlerID, payload, 3);
}
void cpubridge::translate_MSG_FROM_LANGUAGE_TABLE (const rhea::thread::sMsg &msg, u8 *out_tableID, u8 *out_msgRowNum, u8 *out_language1or2)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_MSG_FROM_LANGUAGE_TABLE);
	const u8 *p = (const u8*)msg.buffer;
	*out_tableID = p[0];
	*out_msgRowNum = p[1];
	*out_language1or2 = p[2];
}

//***************************************************
void cpubridge::ask_CPU_ACTIVATE_BUZZER (const sSubscriber &from, u16 handlerID, u8 numRepeat, u8 beepLen_dSec, u8 pausaTraUnBeepELAltro_dSec)
{
	if (numRepeat>15) numRepeat= 15;
	if (beepLen_dSec>15) beepLen_dSec= 15;
	if (pausaTraUnBeepELAltro_dSec>15) pausaTraUnBeepELAltro_dSec= 15;

	u8 otherData[2];
	if (numRepeat < 2)
	{
		otherData[0] = 0;
		otherData[1] = beepLen_dSec;
	}
	else
	{
		otherData[0] = (numRepeat | 0x80);
		otherData[1] = beepLen_dSec | (pausaTraUnBeepELAltro_dSec << 4);
	}
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_ACTIVATE_BUZZER, handlerID, otherData, 2);
}
void cpubridge::translate_CPU_ACTIVATE_BUZZER(const rhea::thread::sMsg &msg, u8 *out_numRepeat, u8 *out_beepLen_dSec, u8 *out_pausaTraUnBeepELAltro_dSec)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_ACTIVATE_BUZZER);
	const u8 *p = (const u8*)msg.buffer;
	*out_numRepeat = p[0] & 0x7F;
	*out_beepLen_dSec = p[1] & 0x0F;
	*out_pausaTraUnBeepELAltro_dSec = ((p[1] & 0xF0) >> 4);
}


//***************************************************
void cpubridge::ask_CPU_BUZZER_STATUS (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_BUZZER_STATUS, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_GET_JUG_CURRENT_REPETITION(const sSubscriber& from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_JUG_CURRENT_REPETITION, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_STOP_JUG(const sSubscriber& from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_JUG, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_IS_QUICK_MENU_PINCODE_SET (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_IS_QUICK_MENU_PINCODE_SET, handlerID);
}

//***************************************************
void cpubridge::notify_MILKER_TYPE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eCPUMilkerType milkerType)
{
	logger->log("notify_MILKER_TYPE\n");

	u8 buffer[4] = { (u8)milkerType, 0, 0, 0 };
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_MILKER_TYPE, handlerID, buffer, 1);
}

//***************************************************
void cpubridge::translateNotify_MILKER_TYPE(const rhea::thread::sMsg &msg, eCPUMilkerType *out_milkerType) 
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_MILKER_TYPE);
	const u8 *p = (const u8*)msg.buffer;
	*out_milkerType = (eCPUMilkerType)p[0];
}

//***************************************************
void cpubridge::ask_CPU_GET_JUG_REPETITIONS(const sSubscriber& from, u16 handlerId)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_JUG_REPETITONS, handlerId);
}

void cpubridge::notify_CPU_GET_JUG_REPETITIONS(const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, const u8 bufLen, const u8* buffer)
{
	logger->log("notify_CPU_GET_JUG_REPETITIONS\n");

	u8	buf[NUM_MAX_SELECTIONS + 1];
	u8	l = bufLen;

	if (l > NUM_MAX_SELECTIONS)
		l = NUM_MAX_SELECTIONS;

	buf[0] = l;
	memcpy(&buf[1], buffer, l);

	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_JUG_REPETITIONS, handlerID, buf, l + 1);
}

void cpubridge::translateNotify_CPU_GET_JUG_REPETITIONS(const rhea::thread::sMsg& msg, u8* out_len, u8* out_buf, u32 sizeof_out_buf)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_JUG_REPETITIONS);
	const u8* p = (const u8*)msg.buffer;
	*out_len = p[0];
	if (*out_len > sizeof_out_buf)
	{
		DBGBREAK;
		*out_len = sizeof_out_buf;
	}
	memcpy(out_buf, &p[1], *out_len);
}


//***************************************************
void cpubridge::ask_CPU_GET_CUPSENSOR_LIVE_VALUE (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_CUPSENSOR_LIVE_VALUE, handlerID);
}

void cpubridge::notify_CPU_GET_CUPSENSOR_LIVE_VALUE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u16 value)
{
	logger->log("notify_CPU_GET_CUPSENSOR_LIVE_VALUE\n");
	u8 optionalData[2];
	rhea::utils::bufferWriteU16(optionalData, value);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_CUPSENSOR_LIVE_VALUE, handlerID, optionalData, 2);
}

void cpubridge::translateNotify_CPU_GET_CUPSENSOR_LIVE_VALUE(const rhea::thread::sMsg &msg, u16 *out_value)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_CUPSENSOR_LIVE_VALUE);
	const u8 *p = (const u8*)msg.buffer;
	*out_value = rhea::utils::bufferReadU16(p);
}



//***************************************************
void cpubridge::notify_CPU_QUERY_ID101 (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u32 id101)
{
	logger->log("notify_CPU_QUERY_ID101\n");
	u8 optionalData[4];
	rhea::utils::bufferWriteU32(optionalData, id101);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_QUERY_ID101, handlerID, optionalData, 4);
}
void cpubridge::translateNotify_CPU_QUERY_ID101(const rhea::thread::sMsg &msg, u32 *out_id101)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_QUERY_ID101);
	const u8 *p = (const u8*)msg.buffer;
	*out_id101 = rhea::utils::bufferReadU32(p);
}

//***************************************************
void cpubridge::notify_CPU_VALIDATE_QUICK_MENU_PINCODE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool bAccepted)
{
	logger->log("notify_CPU_VALIDATE_QUICK_MENU_PINCODE\n");
	u8 optionalData = 0;
	if (bAccepted)
		optionalData = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_VALIDATE_QUICK_MENU_PINCODE, handlerID, &optionalData, 1);
}
void cpubridge::translateNotify_CPU_VALIDATE_QUICK_MENU_PINCODE(const rhea::thread::sMsg &msg, bool *out_bAccepted)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_VALIDATE_QUICK_MENU_PINCODE);
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bAccepted = true;
	else
		*out_bAccepted = false;
}

void cpubridge::notify_MACHINE_LOCK (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eLockStatus lockStatus)
{
	logger->log("notify_MACHINE_LOCK [%d]\n", lockStatus);
	u8 optionalData[2];
	optionalData[0] = static_cast<u8>(lockStatus);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_LOCK_STATUS, handlerID, optionalData, 1);
}
void cpubridge::translateNotify_MACHINE_LOCK(const rhea::thread::sMsg &msg, eLockStatus *out_lockStatus)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_LOCK_STATUS);
	const u8 *p = (const u8*)msg.buffer;
	*out_lockStatus = static_cast<eLockStatus>(p[0]);
}


//***************************************************
void cpubridge::notify_CPU_IS_QUICK_MENU_PINCODE_SET (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool bYes)
{
	logger->log("notify_CPU_IS_QUICK_MENU_PINCODE_SET\n");
	u8 optionalData = 0;
	if (bYes)
		optionalData = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_IS_QUICK_MENU_PINCODE_SET, handlerID, &optionalData, 1);
}
void cpubridge::translateNotify_CPU_IS_QUICK_MENU_PINCODE_SET(const rhea::thread::sMsg &msg, bool *out_bYes)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_IS_QUICK_MENU_PINCODE_SET);
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bYes = true;
	else
		*out_bYes = false;
}

//***************************************************
void cpubridge::notify_CPU_BUZZER_STATUS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool bBuzzerBusy)
{
	logger->log("notify_CPU_BUZZER_STATUS\n");
	u8 optionalData = 0;
	if (bBuzzerBusy)
		optionalData = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_BUZZER_STATUS, handlerID, &optionalData, 1);
}
void cpubridge::translateNotify_CPU_BUZZER_STATUS(const rhea::thread::sMsg &msg, bool *out_bBuzzerBusy)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_BUZZER_STATUS);
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bBuzzerBusy = true;
	else
		*out_bBuzzerBusy = false;
}

//***************************************************
void cpubridge::notify_CPU_STOP_JUG(const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, bool bResult)
{
	logger->log("notify_CPU_STOP_JUG\n");
	u8 optionalData = 0;
	if (bResult)
		optionalData = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_STOP_JUG, handlerID, &optionalData, 1);
}
void cpubridge::translateNotify_CPU_STOP_JUG(const rhea::thread::sMsg& msg, bool *out_bResult)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_STOP_JUG);
	const u8* p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bResult = true;
	else
		*out_bResult = false;
}

//***************************************************
void cpubridge::notify_CPU_GET_JUG_CURRENT_REPETITION(const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, u8 nOf, u8 m)
{
	logger->log("notify_CPU_GET_JUG_CURRENT_REPETITION\n");
	u8 optionalData[2];
	
	optionalData[0] = nOf;
	optionalData[1] = m;
	
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_CPU_GET_JUG_CURRENT_REPETITION, handlerID, &optionalData, 2);
}
void cpubridge::translateNotify_CPU_GET_JUG_CURRENT_REPETITION(const rhea::thread::sMsg& msg, u8* out_nOf, u8* out_m)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_GET_JUG_CURRENT_REPETITION);
	const u8* p = (const u8*)msg.buffer;

	*out_nOf = p[0];
	*out_m = p[1];
}

//***************************************************
void cpubridge::notify_CPU_BROWSER_URL_CHANGE (const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, const char *url)
{
	logger->log("notify_CPU_BROWSER_URL_CHANGE [%s]\n", url);
	
	u8 otherData[512];
	memset (otherData, 0, sizeof(otherData));

	u32 len = rhea::string::utf8::lengthInBytes (reinterpret_cast<const u8*>(url));
	if (len > 200)
		len = 200;

	otherData[0] = static_cast<u8>(len);
	if (len)
		memcpy (&otherData[1], url, len);
	
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_BROWSER_URL_CHANGE, handlerID, &otherData, 1+len);
}
void cpubridge::translateNotify_CPU_BROWSER_URL_CHANGE (const rhea::thread::sMsg& msg, char *out_url, u32 sizeof_out_url)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_BROWSER_URL_CHANGE);
	const u8 *p = (const u8*)msg.buffer;

	u8 len = p[0];
	if (len >= sizeof_out_url)
		len = sizeof_out_url - 1;
	if (len)
		memcpy (out_url, &p[1], len);
	out_url[len] = 0x00;
}

//***************************************************
void cpubridge::ask_CPU_RUN_CAFFE_CORTESIA (const sSubscriber &from, u16 handlerID, u8 macinaNumDa1a4)
{
	u8 optionalData[2];
	optionalData[0] = macinaNumDa1a4;

	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_RUN_CAFFE_CORTESIA, handlerID, optionalData, 1);
}
void cpubridge::translate_CPU_RUN_CAFFE_CORTESIA (const rhea::thread::sMsg &msg, u8 *out_macinaNumDa1a4)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_RUN_CAFFE_CORTESIA);
	const u8 *p = (const u8*)msg.buffer;
	*out_macinaNumDa1a4 = p[0];
}
void cpubridge::notify_CPU_RUN_CAFFE_CORTESIA (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_RUN_CAFFE_CORTESIA\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_RUN_CAFFE_CORTESIA, handlerID);
}

//***************************************************
void cpubridge::notify_END_OF_GRINDER_CLEANING_PROCEDURE (const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger)
{
	logger->log("notify_END_OF_GRINDER_CLEANING_PROCEDURE\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_END_OF_GRINDER_CLEANING_PROC, handlerID);
}

//***************************************************
void cpubridge::notify_CPU_ATTIVAZIONE_SCIVOLO_BREWMATIC (const sSubscriber& to, u16 handlerID, rhea::ISimpleLogger* logger, u8 perc0_100)
{
	logger->log("notify_CPU_ATTIVAZIONE_SCIVOLO_BREWMATIC [%d]\n", perc0_100);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SCIVOLO_BREWMATIC, handlerID);
}

//***************************************************
void cpubridge::ask_SELECTION_ENABLE_DISABLE (const sSubscriber& from, u16 handlerID, u8 selNum, bool bEnable)
{
	u8 buffer[2];
	buffer[0] = selNum;
	buffer[1] = 0x00;
	if (bEnable)
		buffer[1] = 0x01;
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SELECTION_ENABLE, handlerID, buffer, 2);
}
void cpubridge::translate_SELECTION_ENABLE_DISABLE(const rhea::thread::sMsg& msg, u8 *out_selNum, bool *out_enable)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SELECTION_ENABLE);
    const u8* p = (const u8*)msg.buffer;
    *out_selNum = p[0];
	if (p[1] == 0x01)
		*out_enable= true;
	else
		*out_enable= false;
}
void cpubridge::notify_SELECTION_ENABLE_DISABLE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 errorCode)
{
	logger->log("notify_SELECTION_ENABLE_DISABLE[%d]\n", errorCode);
	u8 optionalData[2];
	optionalData[0] = errorCode;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SELECTION_ENABLE, handlerID, optionalData, 1);
}
void cpubridge::translateNotify_SELECTION_ENABLE_DISABLE (const rhea::thread::sMsg &msg, u8 *out_errorCode)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SELECTION_ENABLE);
	const u8 *p = (const u8*)msg.buffer;
	*out_errorCode = p[0];
}

//***************************************************
void cpubridge::ask_OVERWRITE_CPU_MESSAGE_ON_SCREEN (const sSubscriber& from, u16 handlerID, const u8 *msgUTF8, u8 timeSec)
{
	u8 buffer[256];
	memset (buffer, 0, sizeof(buffer));
	buffer[0] = timeSec;
	
	u32 msgLen = rhea::string::utf8::lengthInBytes(msgUTF8);
	if (msgLen > 200)
		msgLen = 200;
	memcpy (&buffer[1], msgUTF8, msgLen);

	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_OVERWRITE_CPU_MESSAGE_ON_SCREEN, handlerID, buffer, 2+msgLen);
}
void cpubridge::translate_OVERWRITE_CPU_MESSAGE_ON_SCREEN(const rhea::thread::sMsg& msg, const u8 **out_msgUTF8, u8 *out_timeSec)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_OVERWRITE_CPU_MESSAGE_ON_SCREEN);
    const u8* p = (const u8*)msg.buffer;
    *out_timeSec = p[0];
	*out_msgUTF8 = &p[1];
}
void cpubridge::notify_OVERWRITE_CPU_MESSAGE_ON_SCREEN (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 errorCode)
{
	logger->log("notify_OVERWRITE_CPU_MESSAGE_ON_SCREEN[%d]\n", errorCode);
	u8 optionalData[2];
	optionalData[0] = errorCode;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_OVERWRITE_CPU_MESSAGE_ON_SCREEN, handlerID, optionalData, 1);
}
void cpubridge::translateNotify_OVERWRITE_CPU_MESSAGE_ON_SCREEN (const rhea::thread::sMsg &msg, u8 *out_errorCode)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_OVERWRITE_CPU_MESSAGE_ON_SCREEN);
	const u8 *p = (const u8*)msg.buffer;
	*out_errorCode = p[0];
}

//***************************************************
void cpubridge::ask_SET_SELECTION_PARAMU16 (const sSubscriber& from, u16 handlerID, u8 selNumDa1aN, eSelectionParam whichParam, u16 paramValue)
{
	u8 buffer[4];
	buffer[0] = selNumDa1aN;
	buffer[1] = static_cast<u8>(whichParam);
	rhea::utils::bufferWriteU16 (&buffer[2], paramValue);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SET_SELECTION_PARAMU16, handlerID, buffer, 4);
}
void cpubridge::translate_SET_SELECTION_PARAMU16 (const rhea::thread::sMsg& msg, u8 *out_selNumDa1aN, eSelectionParam *out_whichParam, u16 *out_paramValue)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_SET_SELECTION_PARAMU16);
    const u8* p = (const u8*)msg.buffer;
    *out_selNumDa1aN = p[0];
	*out_whichParam = static_cast<eSelectionParam>(p[1]);
	*out_paramValue = rhea::utils::bufferReadU16 (&p[2]);
}
void cpubridge::notify_SET_SELECTION_PARAMU16 (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum1ToN, eSelectionParam whichParam, u8 errorCode, u16 paramValue)
{
	logger->log("notify_SET_SELECTION_PARAMU16[sel=%d][paramID=%d][err=%d][val=%d]\n", selNum1ToN,whichParam,errorCode, paramValue);
	u8 optionalData[4];
	optionalData[0] = selNum1ToN;
	optionalData[1] = static_cast<u8>(whichParam);
	optionalData[2] = errorCode;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SET_SELECTION_PARAMU16, handlerID, optionalData, 3);
}
void cpubridge::translateNotify_SET_SELECTION_PARAMU16 (const rhea::thread::sMsg &msg, u8 *out_selNum1ToN, eSelectionParam *out_whichParam, u8 *out_errorCode)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SET_SELECTION_PARAMU16);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum1ToN = p[0];
	*out_whichParam = static_cast<eSelectionParam>(p[1]);
	*out_errorCode = p[2];
}

//***************************************************
void cpubridge::ask_SCHEDULE_ACTION_RELAXED_REBOOT (const sSubscriber& from, u16 handlerID)
{
    rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SCHEDULE_ACTION_RELAXED_REBOOT, handlerID, NULL, 0);
}


//***************************************************
void cpubridge::ask_GET_SELECTION_PARAMU16 (const sSubscriber& from, u16 handlerID, u8 selNumDa1aN, eSelectionParam whichParam)
{
	u8 buffer[4];
	buffer[0] = selNumDa1aN;
	buffer[1] = static_cast<u8>(whichParam);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_GET_SELECTION_PARAMU16, handlerID, buffer, 2);
}
void cpubridge::translate_GET_SELECTION_PARAMU16 (const rhea::thread::sMsg& msg, u8 *out_selNumDa1aN, eSelectionParam *out_whichParam)
{
    assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_GET_SELECTION_PARAMU16);
    const u8* p = (const u8*)msg.buffer;
    *out_selNumDa1aN = p[0];
	*out_whichParam = static_cast<eSelectionParam>(p[1]);
}
void cpubridge::notify_GET_SELECTION_PARAMU16 (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 selNum1ToN, eSelectionParam whichParam, u8 errorCode, u16 paramValue)
{
	logger->log("notify_GET_SELECTION_PARAMU16[sel=%d][paramID=%d][err=%d][val=%d]\n", selNum1ToN,whichParam,errorCode,paramValue);
	u8 optionalData[8];
	optionalData[0] = selNum1ToN;
	optionalData[1] = static_cast<u8>(whichParam);
	optionalData[2] = errorCode;
	rhea::utils::bufferWriteU16 (&optionalData[3], paramValue);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_GET_SELECTION_PARAMU16, handlerID, optionalData, 5);
}
void cpubridge::translateNotify_GET_SELECTION_PARAMU16 (const rhea::thread::sMsg &msg, u8 *out_selNum1ToN, eSelectionParam *out_whichParam, u8 *out_errorCode, u16 *out_paramValue)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_GET_SELECTION_PARAMU16);
	const u8 *p = (const u8*)msg.buffer;
	*out_selNum1ToN = p[0];
	*out_whichParam = static_cast<eSelectionParam>(p[1]);
	*out_errorCode = p[2];
	*out_paramValue = rhea::utils::bufferReadU16 (&p[3]);
}


//***************************************************
void cpubridge::ask_SNACK_ENTER_PROG (const sSubscriber& from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SNACK_ENTER_PROG, handlerID, NULL, 0);
}
void cpubridge::notify_SNACK_ENTER_PROG (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool result)
{
	logger->log("notify_SNACK_ENTER_PROG [%d]\n", result?0:1);
	u8 optionalData[2];
	optionalData[0] = static_cast<u8>(result);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SNACK_ENTER_PROG, handlerID, optionalData, 1);
}
void cpubridge::translateNotify_SNACK_ENTER_PROG(const rhea::thread::sMsg &msg, bool *out_result)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SNACK_ENTER_PROG);
	const u8 *p = (const u8*)msg.buffer;
	*out_result = false;
	if (p[0] == 0x01)
		*out_result = true;
}

//***************************************************
void cpubridge::ask_SNACK_EXIT_PROG (const sSubscriber& from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SNACK_EXIT_PROG, handlerID, NULL, 0);
}
void cpubridge::notify_SNACK_EXIT_PROG (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool result)
{
	logger->log("notify_SNACK_EXIT_PROG [%d]\n", result?0:1);
	u8 optionalData[2];
	optionalData[0] = static_cast<u8>(result);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SNACK_EXIT_PROG, handlerID, optionalData, 1);
}
void cpubridge::translateNotify_SNACK_EXIT_PROG(const rhea::thread::sMsg &msg, bool *out_result)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SNACK_EXIT_PROG);
	const u8 *p = (const u8*)msg.buffer;
	*out_result = false;
	if (p[0] == 0x01)
		*out_result = true;
}

//***************************************************
void cpubridge::ask_SNACK_GET_STATUS (const sSubscriber& from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, CPUBRIDGE_SUBSCRIBER_ASK_SNACK_GET_STATUS, handlerID, NULL, 0);
}
void cpubridge::notify_SNACK_GET_STATUS (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, bool isAlive, const u8 *selStatus1_48)
{
	logger->log("notify_SNACK_GET_STATUS [%d] [%d][%d][%d][%d][%d][%d]\n", isAlive?0:1, selStatus1_48[0], selStatus1_48[1], selStatus1_48[2], selStatus1_48[3], selStatus1_48[4], selStatus1_48[5]);
	u8 optionalData[16];
	optionalData[0] = isAlive ? 0:1;
	memcpy (&optionalData[1], selStatus1_48, 6);
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, CPUBRIDGE_NOTIFY_SNACK_GET_STATUS, handlerID, optionalData, 7);
}
void cpubridge::translateNotify_SNACK_GET_STATUS(const rhea::thread::sMsg &msg, bool *out_isAlive, u8 *out_selStatus1_48, u32 sizeof_outSelStatus)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_SNACK_GET_STATUS);
	const u8 *p = (const u8*)msg.buffer;
	*out_isAlive = false;
	if (p[0] == 0x01)
		*out_isAlive = true;

	u32 n = 6;
	if (sizeof_outSelStatus < n)
		n = sizeof_outSelStatus;
	memcpy (out_selStatus1_48, &p[1], n);
}

