#include "ESAPISubscriberList.h"
#include "../rheaCommonLib/rhea.h"
#include "../CPUBridge/CPUBridgeEnumAndDefine.h"

using namespace esapi;

//**************************************************
SubscriberList::SubscriberList()
{
}

//**************************************************
void SubscriberList::setup (rhea::Allocator *allocator)
{
	list.setup (allocator, 16);
}

//**************************************************
void SubscriberList::unsetup()
{
	list.unsetup ();
}

//**************************************************
sSubscription* SubscriberList::onSubscriptionRequest (rhea::Allocator *localAllocator, rhea::ISimpleLogger *logger, const rhea::thread::sMsg &msg)
{
	logger->log("ESAPI::SubscriberList => new SUBSCRIPTION_REQUEST...");

	//creo la subscription
	sSubscription *sub = RHEAALLOCSTRUCT(localAllocator, sSubscription);
	rhea::thread::createMsgQ (&sub->q.hFromMeToSubscriberR, &sub->q.hFromMeToSubscriberW);
	rhea::thread::createMsgQ (&sub->q.hFromSubscriberToMeR, &sub->q.hFromSubscriberToMeW);
	list.append (sub);

	rhea::thread::getMsgQEvent(sub->q.hFromSubscriberToMeR, &sub->hEvent);

	//rispondo al thread richiedente
	HThreadMsgW hToThreadW;
	hToThreadW.initFromU32 (msg.paramU32);
	rhea::thread::pushMsg (hToThreadW, ESAPI_SERVICECH_SUBSCRIPTION_ANSWER, (u32)0, &sub->q, sizeof(cpubridge::sSubscriber));

	logger->log("OK\n");

	return sub;
}

//**************************************************
void SubscriberList::unsubscribe (rhea::Allocator *localAllocator, sSubscription *sub)
{
	rhea::thread::sMsg msg;
	while (rhea::thread::popMsg (sub->q.hFromSubscriberToMeR, &msg))
		rhea::thread::deleteMsg(msg);
            
	list.findAndRemove(sub);
	RHEAFREE(localAllocator, sub);
}

//**************************************************
sSubscription* SubscriberList::findByOSEvent (const OSEvent &h)
{
	for (u32 i2 = 0; i2 < list.getNElem(); i2++)
	{
		if (rhea::event::compare(list(i2)->hEvent, h))
			return list[i2];
	}

	return NULL;
}