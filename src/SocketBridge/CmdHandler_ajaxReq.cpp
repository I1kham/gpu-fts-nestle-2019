#include "CmdHandler_ajaxReq.h"
#include "CmdHandler/CmdHandler_ajaxReqSelAvailability.h"
#include "CmdHandler/CmdHandler_ajaxReqSelPrices.h"
#include "CmdHandler/CmdHandler_ajaxReqDBC.h"
#include "CmdHandler/CmdHandler_ajaxReqDBQ.h"
#include "CmdHandler/CmdHandler_ajaxReqDBE.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x03_SanWashStatus.h"
#include "CmdHandler/CmdHandler_ajaxReq_T_VMCDataFileTimestamp.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x04_SetDecounter.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x06_GetAllDecounterValues.h"
#include "CmdHandler/CmdHandler_ajaxReqMachineTypeAndModel.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0F_SetCalibFactor.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0B_StatoGruppo.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0C_AttivazioneMotore.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0E_StartImpulseCalc.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x07_GetTime.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x08_GetDate.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x09_SetTime.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x0A_SetDate.h"
#include "CmdHandler/CmdHandler_ajaxReqTestSelection.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x10_GetPosizioneMacina.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x13_NomiLingueCPU.h"
#include "CmdHandler/CmdHandler_ajaxReqFSFileList.h"
#include "CmdHandler/CmdHandler_ajaxReqFSFileCopy.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x16_ResetEVA.h"
#include "CmdHandler/CmdHandler_ajaxReqFSDriveList.h"
#include "CmdHandler/CmdHandler_ajaxReqTaskSpawn.h"
#include "CmdHandler/CmdHandler_ajaxReqTaskStatus.h"
#include "CmdHandler/CmdHandler_ajaxReqDBCloseByPath.h"
#include "CmdHandler/CmdHandler_ajaxReqIsManualInstalled.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x18_GetOFFList.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer.h"
#include "CmdHandler/CmdHandler_ajaxReq_setLastUsedLangForProgMenu.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x1C_StartModemTest.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x1D_ResetEVATotals.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore.h"
#include "CmdHandler/CmdHandler_ajaxReq_M_MilkerVer.h"
#include "CmdHandler/CmdHandler_ajaxReqGetCurSelRunning.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue.h"
#include "CmdHandler/CmdHandler_ajaxReq_AliChina_getQR.h"
#include "CmdHandler/CmdHandler_ajaxReq_AliChina_abort.h"
#include "CmdHandler/CmdHandler_ajaxReq_AliChina_isOnline.h"
#include "CmdHandler/CmdHandler_ajaxReq_AliChina_activate.h"
#include "CmdHandler/CmdHandler_ajaxReq_AliChina_getConnDetail.h"
#include "CmdHandler/CmdHandler_ajaxReqFSRheaUnzip.h"
#include "CmdHandler/CmdHandler_ajaxReqMilkerType.h"
#include "CmdHandler/CmdHandler_ajaxReqJugRepetitions.h"
#include "CmdHandler/CmdHandler_ajaxReqGetGPUVer.h"
#include "CmdHandler/CmdHandler_ajaxReqGetLastInstalledGUIFilename.h"
#include "CmdHandler/CmdHandler_ajaxReqGetLastInstalledCPUFilename.h"
#include "CmdHandler/CmdHandler_ajaxReqGetDA3info.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x25_caffeCortesia.h"
#include "CmdHandler/CmdHandler_ajaxReqFSmkdir.h"
#include "CmdHandler/CmdHandler_ajaxReq_validateQuickMenuPinCode.h"
#include "CmdHandler/CmdHandler_ajaxReq_isQuickMenuPinCodeSet.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x28_getBuzzerStatus.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x2A_stopJug.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x31_askMsgFromLangTable.h"
#include "CmdHandler/CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic.h"

using namespace socketbridge;

/***************************************************
 * Factory
 *
 */
CmdHandler_ajaxReq* CmdHandler_ajaxReqFactory::spawn (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec, const u8 **out_params)
{
    if (payloadLenInBytes < 5)
        return NULL;

    u8 commandLen = payload[0];
    u16 paramLen = ((u16)payload[1 + commandLen] * 256) + (u16)payload[2 + commandLen];

    const char *command = (const char*) &payload[1];
    *out_params = &payload[3 + commandLen];

    payload[1+commandLen] = 0x00;
    payload[3+commandLen+paramLen] = 0x00;

    //printf ("Ajax => reqID=%d, command=%s, params=%s\n", ajaxRequestID, command, *out_params);

    //ora che abbiamo il commandName e i params, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (strcasecmp((const char*)command, TClass::getCommandName()) == 0)\
        return RHEANEW(allocator, TClass)(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID);\
    \


	CHECK(CmdHandler_ajaxReqSelAvailability)
	CHECK(CmdHandler_ajaxReqSelPrices)
	CHECK(CmdHandler_ajaxReqDBC);
	CHECK(CmdHandler_ajaxReqDBQ);
	CHECK(CmdHandler_ajaxReqDBE);
	CHECK(CmdHandler_ajaxReq_P0x03_SanWashStatus);
	CHECK(CmdHandler_ajaxReq_T_VMCDataFileTimestamp);
	CHECK(CmdHandler_ajaxReq_P0x04_SetDecounter);
	CHECK(CmdHandler_ajaxReq_P0x06_GetAllDecounterValues);
	CHECK(CmdHandler_ajaxReqMachineTypeAndModel);
	CHECK(CmdHandler_ajaxReq_P0x0F_SetCalibFactor);
	CHECK(CmdHandler_ajaxReq_P0x0B_StatoGruppo);
	CHECK(CmdHandler_ajaxReq_P0x0C_AttivazioneMotore);
	CHECK(CmdHandler_ajaxReq_P0x0E_StartImpulseCalc);
	CHECK(CmdHandler_ajaxReq_P0x0E_QueryImpulseCalcStatus);
	CHECK(CmdHandler_ajaxReq_P0x07_GetTime);
	CHECK(CmdHandler_ajaxReq_P0x08_GetDate);
	CHECK(CmdHandler_ajaxReq_P0x09_SetTime);
	CHECK(CmdHandler_ajaxReq_P0x0A_SetDate);
	CHECK(CmdHandler_ajaxReqTestSelection);
	CHECK(CmdHandler_ajaxReq_P0x10_GetPosizioneMacina);
	CHECK(CmdHandler_ajaxReq_P0x13_NomiLingueCPU);
    CHECK(CmdHandler_ajaxReqFSFileList);
	CHECK(CmdHandler_ajaxReqFSFileCopy);
	CHECK(CmdHandler_ajaxReq_P0x16_ResetEVA);
	CHECK(CmdHandler_ajaxReqFSDriveList);
	CHECK(CmdHandler_ajaxReqTaskSpawn);
	CHECK(CmdHandler_ajaxReqTaskStatus);
	CHECK(CmdHandler_ajaxReqDBCloseByPath);
	CHECK(CmdHandler_ajaxReqIsManualInstalled);
	CHECK(CmdHandler_ajaxReq_P0x17_GetVoltageAndTemp);
	CHECK(CmdHandler_ajaxReq_P0x18_GetOFFList);
	CHECK(CmdHandler_ajaxReq_P0x19_GetLastFLuxInfo);
	CHECK(CmdHandler_ajaxReq_P0x1B_getCPUStringModelAndVer);
	CHECK(CmdHandler_ajaxReq_setLastUsedLangForProgMenu);
	CHECK(CmdHandler_ajaxReq_P0x1C_StartModemTest);
	CHECK(CmdHandler_ajaxReq_P0x1D_ResetEVATotals);
	CHECK(CmdHandler_ajaxReq_P0x1E_GetTimeLavSanCappuc);
	CHECK(CmdHandler_ajaxReq_P0x1F_StartTestAssorbGruppo);
	CHECK(CmdHandler_ajaxReq_P0x20_getStatusTestAssorbGruppo);
	CHECK(CmdHandler_ajaxReq_P0x21_StartTestAssorbMotoriduttore);
	CHECK(CmdHandler_ajaxReq_P0x22_getStatusTestAssorbMotoriduttore);
	CHECK(CmdHandler_ajaxReq_M_MilkerVer);
	CHECK(CmdHandler_ajaxReqGetCurSelRunning);
	CHECK(CmdHandler_ajaxReq_P0x23_startGrinderSpeedTest);
	CHECK(CmdHandler_ajaxReq_P0x23bis_getLastSpeedValue);
	CHECK(CmdHandler_ajaxReq_AliChina_getQR);
	CHECK(CmdHandler_ajaxReq_AliChina_abort);
	CHECK(CmdHandler_ajaxReq_AliChina_isOnline);
	CHECK(CmdHandler_ajaxReq_AliChina_activate);
	CHECK(CmdHandler_ajaxReq_AliChina_getConnDetail);
	CHECK(CmdHandler_ajaxReqFSRheaUnzip);
	CHECK(CmdHandler_ajaxReqMilkerType);
	CHECK(CmdHandler_ajaxReqJugRepetitions);
	CHECK(CmdHandler_ajaxReqGetGPUVer);
	CHECK(CmdHandler_ajaxReqGetLastInstalledGUIFilename);
	CHECK(CmdHandler_ajaxReqGetLastInstalledCPUFilename);
	CHECK(CmdHandler_ajaxReqGetDA3info);
	CHECK(CmdHandler_ajaxReq_P0x24_getCupSensorLiveValue);
	CHECK(CmdHandler_ajaxReq_P0x25_caffeCortesia);
	CHECK(CmdHandler_ajaxReqFSmkdir);
	CHECK(CmdHandler_ajaxReq_validateQuickMenuPinCode);
	CHECK(CmdHandler_ajaxReq_isQuickMenuPinCodeSet);
	CHECK(CmdHandler_ajaxReq_P0x28_getBuzzerStatus);
	CHECK(CmdHandler_ajaxReq_P0x29_getJugCurrentRepetition);
	CHECK(CmdHandler_ajaxReq_P0x2A_stopJug);
	CHECK(CmdHandler_ajaxReq_P0x2B_notifyEndOfGrinderCleaningProc);
	CHECK(CmdHandler_ajaxReq_P0x31_askMsgFromLangTable);
	CHECK(CmdHandler_ajaxReq_P0x32_ScivoloRotanteBrewmatic);

#undef CHECK
    return NULL;
}
