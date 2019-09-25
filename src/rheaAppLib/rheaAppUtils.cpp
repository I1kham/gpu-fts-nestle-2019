#include "rheaAppUtils.h"


using namespace rhea;
using namespace rhea::app;


static const char UNKNOWN[8] = { "UNKNOWN" };

//***************************************************************
const char*	utils::verbose_eVMCState(cpubridge::eVMCState s)
{
	static const char v[9][16] = {
		{"DISP"},
		{"PREP_BEVANDA"},
		{"PROG"},
		{"INI_CHECK"},
		{"ERROR"},
		{"LAVAGGIO_MAN"},
		{"LAVAGGIO_AUTO"},
		{"RICARICA_ACQUA"},
		{"ATTESA TEMP"}
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
	}
}

//***************************************************************
const char*	utils::verbose_eRunningSelStatus(cpubridge::eRunningSelStatus s)
{
	static const char v[5][32] = {
		{"WAIT"},
		{"RUNNING"},
		{"FINISHED_OK"},
		{"FINISHED_KO"},
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