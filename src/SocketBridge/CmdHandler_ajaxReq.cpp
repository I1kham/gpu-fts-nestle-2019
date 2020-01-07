#include "CmdHandler_ajaxReq.h"
#include "CmdHandler/CmdHandler_ajaxReqSelAvailability.h"
#include "CmdHandler/CmdHandler_ajaxReqSelPrices.h"
#include "CmdHandler/CmdHandler_ajaxReqDBC.h"
#include "CmdHandler/CmdHandler_ajaxReqDBQ.h"
#include "CmdHandler/CmdHandler_ajaxReqDBE.h"
#include "CmdHandler/CmdHandler_ajaxReqSanWashStatus.h"
#include "CmdHandler/CmdHandler_ajaxReqVMCDataFileTimestamp.h"
#include "CmdHandler/CmdHandler_ajaxReqSetDecounter.h"
#include "CmdHandler/CmdHandler_ajaxReqGetAllDecounterValues.h"
#include "CmdHandler/CmdHandler_ajaxReqmachineTypeAndModel.h"
#include "CmdHandler/CmdHandler_ajaxReqSetCalibFactor.h"
#include "CmdHandler/CmdHandler_ajaxReqStatoGruppo.h"
#include "CmdHandler/CmdHandler_ajaxReqAttivazioneMotore.h"
#include "CmdHandler/CmdHandler_ajaxReqStartImpulseCalc.h"
#include "CmdHandler/CmdHandler_ajaxReqQueryImpulseCalcStatus.h"
#include "CmdHandler/CmdHandler_ajaxReqGetTime.h"
#include "CmdHandler/CmdHandler_ajaxReqGetDate.h"
#include "CmdHandler/CmdHandler_ajaxReqSetTime.h"
#include "CmdHandler/CmdHandler_ajaxReqSetDate.h"
#include "CmdHandler/CmdHandler_ajaxReqTestSelection.h"
#include "CmdHandler/CmdHandler_ajaxReqGetPosizioneMacina.h"
#include "CmdHandler/CmdHandler_ajaxReqNomiLingueCPU.h"
#include "CmdHandler/CmdHandler_ajaxReqFSFileList.h"
#include "CmdHandler/CmdHandler_ajaxReqFSFileCopy.h"
#include "CmdHandler/CmdHandler_ajaxReqResetEVA.h"
#include "CmdHandler/CmdHandler_ajaxReqFSDriveList.h"
#include "CmdHandler/CmdHandler_ajaxReqTaskSpawn.h"
#include "CmdHandler/CmdHandler_ajaxReqTaskStatus.h"
#include "CmdHandler/CmdHandler_ajaxReqDBCloseByPath.h"
#include "CmdHandler/CmdHandler_ajaxReqIsManualInstalled.h"
#include "CmdHandler/CmdHandler_ajaxReqGetVoltageAndTemp.h"

using namespace socketbridge;


/***************************************************
 * Factory
 *
 */
CmdHandler_ajaxReq* CmdHandler_ajaxReqFactory::spawn (rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec, const char **out_params)
{
    if (payloadLenInBytes < 5)
        return NULL;

    u8 commandLen = payload[0];
    u16 paramLen = ((u16)payload[1 + commandLen] * 256) + (u16)payload[2 + commandLen];

    const char *command = (const char*) &payload[1];
    *out_params = (const char*) &payload[3 + commandLen];

    payload[1+commandLen] = 0x00;
    payload[3+commandLen+paramLen] = 0x00;

    //printf ("Ajax => reqID=%d, command=%s, params=%s\n", ajaxRequestID, command, *out_params);

    //ora che abbiamo il commandName e i params, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (strcasecmp(command, TClass::getCommandName()) == 0)\
        return RHEANEW(allocator, TClass)(identifiedClientHandle, handlerID, dieAfterHowManyMSec, ajaxRequestID);\
    \


	CHECK(CmdHandler_ajaxReqSelAvailability)
	CHECK(CmdHandler_ajaxReqSelPrices)
	CHECK(CmdHandler_ajaxReqDBC);
	CHECK(CmdHandler_ajaxReqDBQ);
	CHECK(CmdHandler_ajaxReqDBE);
	CHECK(CmdHandler_ajaxReqSanWashStatus);
	CHECK(CmdHandler_ajaxReqVMCDataFileTimestamp);
	CHECK(CmdHandler_ajaxReqSetDecounter);
	CHECK(CmdHandler_ajaxReqGetAllDecounterValues);
	CHECK(CmdHandler_ajaxReqmachineTypeAndModel);
	CHECK(CmdHandler_ajaxReqSetCalibFactor);
	CHECK(CmdHandler_ajaxReqStatoGruppo);
	CHECK(CmdHandler_ajaxReqAttivazioneMotore);
	CHECK(CmdHandler_ajaxReqStartImpulseCalc);
	CHECK(CmdHandler_ajaxReqQueryImpulseCalcStatus);
	CHECK(CmdHandler_ajaxReqGetTime);
	CHECK(CmdHandler_ajaxReqGetDate);
	CHECK(CmdHandler_ajaxReqSetTime);
	CHECK(CmdHandler_ajaxReqSetDate);
	CHECK(CmdHandler_ajaxReqTestSelection);
	CHECK(CmdHandler_ajaxReqGetPosizioneMacina);
	CHECK(CmdHandler_ajaxReqNomiLingueCPU);
    CHECK(CmdHandler_ajaxReqFSFileList);
	CHECK(CmdHandler_ajaxReqFSFileCopy);
	CHECK(CmdHandler_ajaxReqResetEVA);
	CHECK(CmdHandler_ajaxReqFSDriveList);
	CHECK(CmdHandler_ajaxReqTaskSpawn);
	CHECK(CmdHandler_ajaxReqTaskStatus);
	CHECK(CmdHandler_ajaxReqDBCloseByPath);
	CHECK(CmdHandler_ajaxReqIsManualInstalled);
	CHECK(CmdHandler_ajaxReqGetVoltageAndTemp);

#undef CHECK
    return NULL;
}
