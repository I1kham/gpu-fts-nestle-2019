#ifdef WIN32
#include <string.h>
#include "winOSWaitableGrp.h"
#include "../../rheaMemory.h"


//***********************************************
OSWaitableGrp::OSWaitableGrp()
{
    base = NULL;
	nEventsReady = 0;
	for (int i = 0; i < MAX_EVENTS_PER_CALL; i++)
		events[i] = INVALID_HANDLE_VALUE;
}

//***********************************************
OSWaitableGrp::~OSWaitableGrp()
{
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    while (base)
    {
        sRecord *p = base;
        base = base->next;

        allocator->dealloc(p);
    }
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_newRecord ()
{
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    sRecord *r = RHEAALLOCSTRUCT(allocator,sRecord);
    r->next = base;
    base = r;
    return r;
}

//***********************************************
void OSWaitableGrp::priv_removeHandle (HANDLE h)
{
	for (int i = 0; i < MAX_EVENTS_PER_CALL; i++)
	{
		if (events[i] == h)
		{
			u32 nToCopy = MAX_EVENTS_PER_CALL - (i+1);
			if (nToCopy)
				memcpy(&events[i], &events[i + 1], sizeof(HANDLE) * nToCopy);
			events[MAX_EVENTS_PER_CALL-1] = INVALID_HANDLE_VALUE;
			return;
		}
	}
}

//***********************************************
void OSWaitableGrp::removeSocket (OSSocket &sok)
{ 
	rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

	sRecord *q = NULL;
	sRecord *p = base;
	while (p)
	{
		if (p->originType == evt_origin_socket)
		{
			if (sok.hEventNotify == p->origin.osSocket.sok.hEventNotify)
			{
				priv_removeHandle (sok.hEventNotify);
				WSACloseEvent(sok.hEventNotify);
				sok.hEventNotify = INVALID_HANDLE_VALUE;

				if (q == NULL)
				{
					base = base->next;
					allocator->dealloc(p);
					return;
				}

				q->next = p->next;
				allocator->dealloc(p);
				return;
			}
		}

		q = p;
		p = p->next;
	}
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_addSocket (OSSocket &sok)
{
	assert(sok.hEventNotify == INVALID_HANDLE_VALUE);

	sok.hEventNotify = WSACreateEvent();
	//WSAEventSelect(sok.socketID, sok.hEventNotify, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE);
	WSAEventSelect(sok.socketID, sok.hEventNotify, FD_READ | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE);

	sRecord *s = priv_newRecord();
    s->originType = (u8)evt_origin_socket;
    s->origin.osSocket.sok = sok;
    return s;
}


//***********************************************
void OSWaitableGrp::removeEvent (const OSEvent &evt)
{
	rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

	sRecord *q = NULL;
	sRecord *p = base;
	while (p)
	{
		if (p->originType == evt_origin_osevent)
		{
			if (OSEvent_compare(p->origin.osEvent.evt, evt))
			{
				priv_removeHandle(p->origin.osEvent.evt.h);

				if (q == NULL)
				{
					base = base->next;
					allocator->dealloc(p);
					return;
				}

				q->next = p->next;
				allocator->dealloc(p);
				return;
			}
		}

		q = p;
		p = p->next;
	}
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_addEvent (const OSEvent &evt)
{
	sRecord *s = priv_newRecord();
	s->originType = (u8)evt_origin_osevent;
    s->origin.osEvent.evt = evt;
    return s;
}

//***********************************************
u8 OSWaitableGrp::wait (u32 timeoutMSec)
{
//per printare un po' di info di debug, definisci la seguente
#undef OSWAITABLE_GRP_DEBUG_TEXT


#ifdef OSWAITABLE_GRP_DEBUG_TEXT
	#define DEBUG_PRINTF(...) printf(__VA_ARGS__)
	#define ADD_EVENT_AND_DEBUG_TEXT(p, debug_text) { generatedEventList[nEventsReady++] = p; printf("  wait::" debug_text "\n"); }
#else
	#define DEBUG_PRINTF(...) {}
	#define ADD_EVENT_AND_DEBUG_TEXT(p, debug_text) { generatedEventList[nEventsReady++] = p;}
#endif


	for (int i = 0; i < MAX_EVENTS_PER_CALL; i++)
		events[i] = INVALID_HANDLE_VALUE;

	DWORD n = 0;
	sRecord *p = base;
	while (p)
	{
		switch (p->originType)
		{
			default:
				DBGBREAK;
				break;

		case (u8)evt_origin_socket:
			events[n++] = p->origin.osSocket.sok.hEventNotify;
			break;

		case (u8)evt_origin_osevent:
			events[n++] = p->origin.osEvent.evt.h;
			break;
		}
		
		p = p->next;
	}

	DWORD dwMilliseconds = (DWORD)timeoutMSec;
	if (timeoutMSec == u32MAX)
		dwMilliseconds = INFINITE;

	DEBUG_PRINTF("\n\nOSWaitableGrp::wait\n");
	DEBUG_PRINTF("  n=%d\n", n);
	for (int i = 0; i < MAX_EVENTS_PER_CALL; i++)
		DEBUG_PRINTF("  %X", events[i]);

	nEventsReady = 0;
	DWORD ret = WaitForMultipleObjects(n, events, FALSE, dwMilliseconds);

	DEBUG_PRINTF("  ret=%d\n", ret);

	if (ret == WAIT_TIMEOUT)
	{
		DEBUG_PRINTF("  WAIT_TIMEOUT\n", ret);
		return 0;
	}
		

	if (ret == WAIT_FAILED)
	{
		DBGBREAK;
		return 0;
	}

	

	DWORD index;
	if (ret >= WAIT_ABANDONED_0)
		index = ret - WAIT_ABANDONED_0;
	else
		index = ret - WAIT_OBJECT_0;
		

	//cerco l'handle
	p = base;
	while (p)
	{
		bool bFound = false;
		switch (p->originType)
		{
		default:
			DBGBREAK;
			break;

		case (u8)evt_origin_socket:
			if (events[index] == p->origin.osSocket.sok.hEventNotify)
				bFound = true;
			break;

		case (u8)evt_origin_osevent:
			if (events[index] == p->origin.osEvent.evt.h)
				bFound = true;
			break;
		}

		if (bFound)
		{
			if (p->originType == evt_origin_socket)
			{
				
				WSANETWORKEVENTS networkEvents;
				//WSAEnumNetworkEvents(p->origin.osSocket.sok.socketID, p->origin.osSocket.sok.hEventNotify, &networkEvents);
				WSAEnumNetworkEvents(p->origin.osSocket.sok.socketID, NULL, &networkEvents);

				if (networkEvents.lNetworkEvents == 0)
					::ResetEvent(p->origin.osSocket.sok.hEventNotify);
				

				DEBUG_PRINTF("  was a socket [userparam=%d], event bits = 0x%X\n", p->userParam.asU32, networkEvents.lNetworkEvents);
				if ((networkEvents.lNetworkEvents & FD_READ) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_READ")
				if ((networkEvents.lNetworkEvents & FD_WRITE) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_WRITE")
				if ((networkEvents.lNetworkEvents & FD_OOB) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_OOB")
				if ((networkEvents.lNetworkEvents & FD_ACCEPT) != 0)					ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ACCEPT")
				if ((networkEvents.lNetworkEvents & FD_CONNECT) != 0)					ADD_EVENT_AND_DEBUG_TEXT(p, "FD_CONNECT")
				if ((networkEvents.lNetworkEvents & FD_CLOSE) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_CLOSE")
				if ((networkEvents.lNetworkEvents & FD_QOS) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_QOS")
				if ((networkEvents.lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE) != 0)	ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ROUTING_INTERFACE_CHANGE")
				if ((networkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE) != 0)		ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ADDRESS_LIST_CHANGE")
			}
			else if (p->originType == evt_origin_osevent)
			{
				generatedEventList[nEventsReady++] = p;
			}

			return nEventsReady;
		}
		p = p->next;
	}

	DEBUG_PRINTF("  handle not found\n", ret);

	return 0;

#undef DEBUG_PRINTF
#undef ADD_EVENT_AND_DEBUG_TEXT
}

//***********************************************
OSWaitableGrp::eEventOrigin OSWaitableGrp::getEventOrigin (u8 iEvent) const
{
    assert (iEvent < nEventsReady);
	return (eEventOrigin)generatedEventList[iEvent]->originType;
}

//***********************************************
void* OSWaitableGrp::getEventUserParamAsPtr (u8 iEvent) const
{
	assert(iEvent < nEventsReady);
	return generatedEventList[iEvent]->userParam.asPtr;
}

//***********************************************
u32 OSWaitableGrp::getEventUserParamAsU32 (u8 iEvent) const
{
	assert(iEvent < nEventsReady);
	return generatedEventList[iEvent]->userParam.asU32;
}

//***********************************************
OSSocket& OSWaitableGrp::getEventSrcAsOSSocket (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_socket);
    return generatedEventList[iEvent]->origin.osSocket.sok;
}

//***********************************************
OSEvent& OSWaitableGrp::getEventSrcAsOSEvent (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_osevent);
	return generatedEventList[iEvent]->origin.osEvent.evt;
}

/***********************************************
OSSerialPort& OSWaitableGrp::getEventSrcAsOSSerialPort (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_serialPort);
    return currentEvent->origin.osSerialPort;
}
*/
#endif