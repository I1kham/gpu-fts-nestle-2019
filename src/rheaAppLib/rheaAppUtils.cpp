#include "rheaAppUtils.h"


using namespace rhea;
using namespace rhea::app;


static const char UNKNOWN[8] = { "UNKNOWN" };

//***************************************************************
const char*	utils::verbose_eVMCState(cpubridge::eVMCState s)
{
    static const char v[19][16] = {
        {"READY"},      //DISPONIBILE
        {"DISPENSING"}, //PREP BEVANDA
		{"PROG"},
		{"INI_CHECK"},
		{"ERROR"},
        {"WASHING_MAN"},    //5
        {"WASHING_AUTO"},
        {"WATER_RECHARGE"},
        {"WAIT_TEMP"},
		{"CARICA_MASTER"},
        {"INSTALL"},        //10
        {"DISINSTALL"},
		{"FINE_INSTALL"},
		{"FINE_DISINST"},
		{"ENERGY_SAVING"},
        {"TEST_DB"},        //15
		{"DATA_AUDIT"},
        {"WASHING_SAN"},    //17
		{"COM_ERROR"},
	};

	switch (s)
	{
	default:											return UNKNOWN;
	case cpubridge::eVMCState_DISPONIBILE:				return v[0];
	case cpubridge::eVMCState_PREPARAZIONE_BEVANDA:		return v[1];
	case cpubridge::eVMCState_PROGRAMMAZIONE:			return v[2];
	case cpubridge::eVMCState_INITIAL_CHECK:			return v[3];
	case cpubridge::eVMCState_ERROR:					return v[4];
	case cpubridge::eVMCState_LAVAGGIO_MANUALE:			return v[5];
	case cpubridge::eVMCState_LAVAGGIO_AUTO:			return v[6];
	case cpubridge::eVMCState_RICARICA_ACQUA:			return v[7];
	case cpubridge::eVMCState_ATTESA_TEMPERATURA:		return v[8];

	case cpubridge::eVMCState_ATTESA_CARICA_MASTER:		return v[9];
	case cpubridge::eVMCState_INSTALLAZIONE:			return v[10];
	case cpubridge::eVMCState_DISINSTALLAZIONE:			return v[11];
	case cpubridge::eVMCState_FINE_INSTALLAZIONE:		return v[12];
	case cpubridge::eVMCState_FINE_DISINSTALLAZIONE:	return v[13];
	case cpubridge::eVMCState_ENERGY_SAVING:			return v[14];
    case cpubridge::eVMCState_TEST_DB:                  return v[15];
	case cpubridge::eVMCState_DATA_AUDIT:				return v[16];
    case cpubridge::eVMCState_LAVAGGIO_SANITARIO:		return v[17];

    case cpubridge::eVMCState_COM_ERROR:				return v[18];
	}
}

//***************************************************************
const char*	utils::verbose_eRunningSelStatus(cpubridge::eRunningSelStatus s)
{
	static const char v[5][32] = {
		{"WAIT"},
		{"RUNNING"},
		{"FINISHED_KO"},
		{"FINISHED_OK"},
		{"RUNNING_CAN_USE_BNT_STOP"}
	};

	switch (s)
	{
	default:												return UNKNOWN;
	case cpubridge::eRunningSelStatus_wait:					return v[0];
	case cpubridge::eRunningSelStatus_running:				return v[1];
	case cpubridge::eRunningSelStatus_finished_KO:			return v[2];
	case cpubridge::eRunningSelStatus_finished_OK:			return v[3];
	case cpubridge::eRunningSelStatus_runningCanUseStopBtn:	return v[4];
	}

}

//***************************************************************
void utils::verbose_SokBridgeClientVer(const socketbridge::SokBridgeClientVer &s, char *out, u32 sizeOfOut)
{
	char sAppType[32];
	
	switch (s.appType)
	{
	case socketbridge::SokBridgeClientVer::APP_TYPE_CONSOLE:
		sprintf_s(sAppType, sizeof(sAppType), "CONSOLE");
		break;

	case socketbridge::SokBridgeClientVer::APP_TYPE_GUI:
		sprintf_s(sAppType, sizeof(sAppType), "GUI");
		break;

	default:
		sprintf_s(sAppType, sizeof(sAppType), "UNKNOWN");
		break;
	}

	sprintf_s(out, sizeOfOut, "%s api_ver:0x%02X [0x%02X] [0x%02X]", sAppType, s.apiVersion, s.unused2, s.unused3);
}

//***************************************************************
const char*	utils::verbose_fileTransferStatus(eFileTransferStatus s)
{
	static const char v[3][16] = {
		{"FINISHED_OK"},
		{"FINISHED_KO"},
		{"IN_PROGRESS"}
	};

	switch (s)
	{
	default:								return UNKNOWN;
	case eFileTransferStatus_finished_OK:	return v[0];
	case eFileTransferStatus_finished_KO:	return v[1];
	case eFileTransferStatus_inProgress:	return v[2];
	}
}

//***************************************************************
const char*	utils::verbose_fileTransferFailReason(socketbridge::eFileTransferFailReason s)
{
	static const char v[6][32] = {
		{"NONE"},
		{"TIMEOUT"},
		{"SMU_REFUSED"},
		{"READBUF_TOO_SHORT"},
		{"SMU-ERR OPENING FILE"},
		{"SMU-FILE TOO BIG OR EMPTY"}
	};

	switch (s)
	{
	default:														return UNKNOWN;
	case socketbridge::eFileTransferFailReason_none:				return v[0];
	case socketbridge::eFileTransferFailReason_timeout:				return v[1];
	case socketbridge::eFileTransferFailReason_smuRefused:			return v[2];
	case socketbridge::eFileTransferFailReason_localReadBufferTooShort: return v[3];
	case socketbridge::eFileTransferFailReason_smuErrorOpeningFile: return v[4];
	case socketbridge::eFileTransferFailReason_smuFileTooBigOrEmpty: return v[5];
	}
}	


//***************************************************************
const char* utils::verbose_readDataFileStatus(cpubridge::eReadDataFileStatus status)
{
	static const char v[5][40] = {
		{"IN_PROGRESS"},
		{"FINISHED_OK"},
		{"FINISHED_KO_CANT_START_INVALID_STATE"},
		{"FINISHED_KO_CPU_NO_ANSWER"},
		{"FINISHED_KO_CPU_CANT_CREATE_FILE"},
	};

	switch (status)
	{
	default:																	return UNKNOWN;
	case cpubridge::eReadDataFileStatus_inProgress:								return v[0];
	case cpubridge::eReadDataFileStatus_finishedOK:								return v[1];
	case cpubridge::eReadDataFileStatus_finishedKO_cantStart_invalidState:		return v[2];
	case cpubridge::eReadDataFileStatus_finishedKO_cpuDidNotAnswer:				return v[3];
	case cpubridge::eReadDataFileStatus_finishedKO_unableToCreateFile:			return v[4];
	}
}


//***************************************************************
const char* utils::verbose_writeDataFileStatus(cpubridge::eWriteDataFileStatus status)
{
	static const char v[6][40] = {
		{"IN_PROGRESS"},
		{"FINISHED_OK"},
		{"FINISHED_KO_CANT_START_INVALID_STATE"},
		{"FINISHED_KO_CPU_NO_ANSWER"},
		{"FINISHED_KO_CPU_CANT_COPY_LOCAL_FILE"},
		{"FINISHED_KO_CPU_CANT_OPEN_LOCAL_FILE"},
	};

	switch (status)
	{
	default:																	return UNKNOWN;
	case cpubridge::eWriteDataFileStatus_inProgress:							return v[0];
	case cpubridge::eWriteDataFileStatus_finishedOK:							return v[1];
	case cpubridge::eWriteDataFileStatus_finishedKO_cantStart_invalidState:		return v[2];
	case cpubridge::eWriteDataFileStatus_finishedKO_cpuDidNotAnswer:			return v[3];
	case cpubridge::eWriteDataFileStatus_finishedKO_unableToCopyFile:			return v[4];
	case cpubridge::eWriteDataFileStatus_finishedKO_unableToOpenLocalFile:		return v[5];
	}
}

//***************************************************************
const char* utils::verbose_WriteCPUFWFileStatus (cpubridge::eWriteCPUFWFileStatus status)
{
	static const char v[10][40] = {
		{"ERASING FLASH"},
		{"IN_PROGRESS"},
		{"FINISHED_OK"},
		{"FINISHED_KO_CANT_START_INVALID_STATE"}, //3
		{"FINISHED_KO_CPU_CANT_COPY_LOCAL_FILE"},
		{"FINISHED_KO_CPU_CANT_OPEN_LOCAL_FILE"},
		{"FINISHED_KO_k_notReceived"}, //6
		{"FINISHED_KO_M_notReceived"}, //7
		{"FINISHED_KO_h_notReceived"}, //8
		{"FINISHED_KO_GENERAL_ERROR"} //9
	};

	switch (status)
	{
	default:																	return UNKNOWN;
	case cpubridge::eWriteCPUFWFileStatus_inProgress_erasingFlash:				return v[0];
	case cpubridge::eWriteCPUFWFileStatus_inProgress: 							return v[1];
	case cpubridge::eWriteCPUFWFileStatus_finishedOK: 							return v[2];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_cantStart_invalidState:	return v[3];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_unableToCopyFile:			return v[4];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_unableToOpenLocalFile:		return v[5];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_k_notReceived: 			return v[6];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_M_notReceived: 			return v[7];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_h_notReceived: 			return v[8];
	case cpubridge::eWriteCPUFWFileStatus_finishedKO_generalError: 				return v[9];
	}
}
