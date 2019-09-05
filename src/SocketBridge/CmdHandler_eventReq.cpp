#include "CmdHandler_eventReq.h"
#include "SocketBridgeEnumAndDefine.h"
#include "CmdHandler/CmdHandler_eventReqCPUMessage.h"
#include "CmdHandler/CmdHandler_eventReqCPUStatus.h"
#include "CmdHandler/CmdHandler_eventReqCreditUpdated.h"
#include "CmdHandler/CmdHandler_eventReqSelAvailability.h"
#include "CmdHandler/CmdHandler_eventReqSelPrices.h"
#include "CmdHandler/CmdHandler_eventReqSelStatus.h"
#include "CmdHandler/CmdHandler_eventReqStartSel.h"
#include "CmdHandler/CmdHandler_eventReqStopSel.h"


using namespace socketbridge;

/***************************************************
 * Factory
 *
 */
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawnFromSocketClientEventType(rhea::Allocator *allocator, const HSokServerClient &hClient, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec)
{
    //ora che abbiamo l'eventtype, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (eventType == TClass::EVENT_TYPE_FROM_SOCKETCLIENT)\
        return RHEANEW(allocator, TClass)(hClient, handlerID, dieAfterHowManyMSec);\


    CHECK(CmdHandler_eventReqCPUMessage)
	CHECK(CmdHandler_eventReqCPUStatus)
	CHECK(CmdHandler_eventReqCreditUpdated)
	CHECK(CmdHandler_eventReqSelAvailability)
	CHECK(CmdHandler_eventReqSelPrices)
	CHECK(CmdHandler_eventReqSelStatus)
    CHECK(CmdHandler_eventReqStartSel)
    CHECK(CmdHandler_eventReqStopSel)
    

#undef CHECK

    return NULL;
}

//***************************************************
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawnFromCPUBridgeEventID(rhea::Allocator *allocator, const HSokServerClient &hClient, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec)
{
#define CHECK(TClass) \
    if (eventID == TClass::EVENT_ID_FROM_CPUBRIDGE)\
        return RHEANEW(allocator, TClass)(hClient, handlerID, dieAfterHowManyMSec);\

	CHECK(CmdHandler_eventReqCPUMessage)
	CHECK(CmdHandler_eventReqCPUStatus)
	CHECK(CmdHandler_eventReqCreditUpdated)
	CHECK(CmdHandler_eventReqSelAvailability)
	CHECK(CmdHandler_eventReqSelPrices)
	CHECK(CmdHandler_eventReqSelStatus)
	CHECK(CmdHandler_eventReqStartSel)
	CHECK(CmdHandler_eventReqStopSel)


#undef CHECK

	return NULL;

}