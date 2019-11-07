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
#undef CHECK
    return NULL;
}
