#include "CmdHandler_ajaxReq.h"
#include "CmdHandler/CmdHandler_ajaxReqSelAvailability.h"
#include "CmdHandler/CmdHandler_ajaxReqSelPrices.h"

using namespace guibridge;


/***************************************************
 * Factory
 *
 */
CmdHandler_ajaxReq* CmdHandler_ajaxReqFactory::spawn (rhea::Allocator *allocator, const HWebsokClient &hClient, u8 ajaxRequestID, u8 *payload, u16 payloadLenInBytes, u16 handlerID, u64 dieAfterHowManyMSec, const char **out_params)
{
    if (payloadLenInBytes < 5)
        return NULL;

    u8 commandLen = payload[0];
    u16 paramLen = ((u16)payload[1 + commandLen] * 256) + (u16)payload[2 + commandLen];

    const char *command = (const char*) &payload[1];
    *out_params = (const char*) &payload[3 + commandLen];

    payload[1+commandLen] = 0x00;
    payload[3+commandLen+paramLen] = 0x00;

    printf ("Ajax => reqID=%d, command=%s, params=%s\n", ajaxRequestID, command, *out_params);

    //ora che abbiamo il commandName e i params, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (strcasecmp(command, TClass::getCommandName()) == 0)\
        return RHEANEW(allocator, TClass)(hClient, handlerID, dieAfterHowManyMSec, ajaxRequestID);\
    \


    CHECK(CmdHandler_ajaxReqSelAvailability)
    CHECK(CmdHandler_ajaxReqSelPrices)


#undef CHECK
    return NULL;
}
