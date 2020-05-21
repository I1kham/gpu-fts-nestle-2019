#include "CmdHandler_eventReq.h"
#include "../SocketBridge/SocketBridgeEnumAndDefine.h"


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

#undef CHECK

    return NULL;
}

//***************************************************
CmdHandler_eventReq* CmdHandler_eventReqFactory::spawnFromCPUBridgeEventID(rhea::Allocator *allocator, const HSokBridgeClient &identifiedClientHandle, u16 eventID, u16 handlerID, u64 dieAfterHowManyMSec)
{
#define CHECK(TClass) \
    if (eventID == TClass::EVENT_ID_FROM_CPUBRIDGE)\
        return RHEANEW(allocator, TClass)(identifiedClientHandle, handlerID, dieAfterHowManyMSec);\

	
#undef CHECK

	return NULL;

}
