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
#include "CmdHandler/CmdHandler_eventReqClientList.h"
#include "CmdHandler/CmdHandler_eventReqCPUbtnProgPressed.h"
#include "CmdHandler/CmdHandler_eventReqDataAudit.h"
#include "CmdHandler/CmdHandler_eventReqCPUIniParam.h"
#include "CmdHandler/CmdHandler_eventReqVMCDataFile.h"
#include "CmdHandler/CmdHandler_eventReqVMCDataFileTimestamp.h"
#include "CmdHandler/CmdHandler_eventReqWriteLocalVMCDataFile.h"
#include "CmdHandler/CmdHandler_eventReqCPUProgrammingCmd.h"
#include "CmdHandler/CmdHandler_eventReqCPUSanWashingStatus.h"
#include "CmdHandler/CmdHandler_eventReqBtnPressed.h"
#include "CmdHandler/CmdHandler_eventReqPartialVMCDataFile.h"

using namespace socketbridge;

/***************************************************
 * Factory
 *
 */
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawnFromSocketClientEventType(rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, eEventType eventType, u16 handlerID, u64 dieAfterHowManyMSec)
{
    //ora che abbiamo l'eventtype, cerchiamo la classe che gestisce questo evento
#define CHECK(TClass) \
    if (eventType == TClass::EVENT_TYPE_FROM_SOCKETCLIENT)\
        return RHEANEW(allocator, TClass)(identifiedClientHandle, handlerID, dieAfterHowManyMSec);\


	CHECK(CmdHandler_eventReqCPUMessage)
	CHECK(CmdHandler_eventReqCPUStatus)
	CHECK(CmdHandler_eventReqCreditUpdated)
	CHECK(CmdHandler_eventReqSelAvailability)
	CHECK(CmdHandler_eventReqSelPrices)
	CHECK(CmdHandler_eventReqSelStatus)
	CHECK(CmdHandler_eventReqStartSel)
	CHECK(CmdHandler_eventReqStopSel)
	CHECK(CmdHandler_eventReqClientList);
	CHECK(CmdHandler_eventReqDataAudit);
	CHECK(CmdHandler_eventReqCPUIniParam);
	CHECK(CmdHandler_eventReqVMCDataFile);
	CHECK(CmdHandler_eventReqVMCDataFileTimestamp);
	CHECK(CmdHandler_eventReqWriteLocalVMCDataFile);
	CHECK(CmdHandler_eventReqCPUProgrammingCmd);
	CHECK(CmdHandler_eventReqCPUSanWashingStatus);
	CHECK(CmdHandler_eventReqBtnPressed);
	CHECK(CmdHandler_eventReqPartialVMCDataFile);

#undef CHECK

    return NULL;
}

//***************************************************
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawnFromCPUBridgeEventID(rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec)
{
#define CHECK(TClass) \
    if (eventID == TClass::EVENT_ID_FROM_CPUBRIDGE)\
        return RHEANEW(allocator, TClass)(identifiedClientHandle, handlerID, dieAfterHowManyMSec);\

	CHECK(CmdHandler_eventReqCPUMessage)
	CHECK(CmdHandler_eventReqCPUStatus)
	CHECK(CmdHandler_eventReqCreditUpdated)
	CHECK(CmdHandler_eventReqSelAvailability)
	CHECK(CmdHandler_eventReqSelPrices)
	CHECK(CmdHandler_eventReqSelStatus)
	CHECK(CmdHandler_eventReqStartSel)
	CHECK(CmdHandler_eventReqStopSel)
	CHECK(CmdHandler_eventReqClientList);
	CHECK(CmdHandler_eventReqCPUBtnProgPressed);
	CHECK(CmdHandler_eventReqDataAudit);
	CHECK(CmdHandler_eventReqCPUIniParam);
	CHECK(CmdHandler_eventReqVMCDataFile);
	CHECK(CmdHandler_eventReqVMCDataFileTimestamp);
	CHECK(CmdHandler_eventReqWriteLocalVMCDataFile);
	CHECK(CmdHandler_eventReqCPUProgrammingCmd);
	CHECK(CmdHandler_eventReqCPUSanWashingStatus);
	CHECK(CmdHandler_eventReqBtnPressed);
	CHECK(CmdHandler_eventReqPartialVMCDataFile);
#undef CHECK

	return NULL;

}
