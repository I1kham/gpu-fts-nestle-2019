#include "CmdHandler_eventReq.h"
#include "GUIBridgeEnumAndDefine.h"
#include "CmdHandler/CmdHandler_eventReqSelAvailability.h"
#include "CmdHandler/CmdHandler_eventReqSelPrices.h"
#include "CmdHandler/CmdHandler_eventReqSelStatus.h"
#include "CmdHandler/CmdHandler_eventReqStartSel.h"
#include "CmdHandler/CmdHandler_eventReqCPUMessage.h"
#include "CmdHandler/CmdHandler_eventReqStopSel.h"
#include "CmdHandler/CmdHandler_eventReqCreditUpdated.h"

using namespace guibridge;

/***************************************************
 * Factory
 *
 */
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawn (rhea::Allocator *allocator, const HWebsokClient &hClient, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec)
{
    //ora che abbiamo l'eventtype, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (eventType == TClass::EVENT_TYPE)\
        return RHEANEW(allocator, TClass)(hClient, handlerID, dieAfterHowManyMSec);\


    CHECK(CmdHandler_eventReqCPUMessage)
    CHECK(CmdHandler_eventReqStartSel)
    CHECK(CmdHandler_eventReqStopSel)
    CHECK(CmdHandler_eventReqSelAvailability)
    CHECK(CmdHandler_eventReqSelPrices)
    CHECK(CmdHandler_eventReqSelStatus)
    CHECK(CmdHandler_eventReqCreditUpdated)


#undef CHECK

    return NULL;
}
