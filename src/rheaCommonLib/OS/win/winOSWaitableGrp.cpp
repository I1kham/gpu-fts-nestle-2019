#ifdef WIN32
#include <string.h>
#include "winOSWaitableGrp.h"
#include "../../rheaMemory.h"


//***********************************************
OSWaitableGrp::OSWaitableGrp()
{
	debug_bWaiting = 0;
    base = NULL;
	nEventsReady = 0;
	for (int i = 0; i < MAX_EVENTS_HANDLE_PER_CALL; i++)
		eventsHandle[i] = INVALID_HANDLE_VALUE;
}

//***********************************************
OSWaitableGrp::~OSWaitableGrp()
{
	rhea::Allocator *allocator = rhea::memory_getSysHeapAllocator();
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
    rhea::Allocator *allocator = rhea::memory_getSysHeapAllocator();
    sRecord *r = RHEAALLOCSTRUCT(allocator,sRecord);
    r->next = base;
    base = r;
    return r;
}

//***********************************************
void OSWaitableGrp::priv_removeHandle (HANDLE h)
{
	/*
	non serve dato che eventsHandle viene ribuildato ad ogni wait
	for (int i = 0; i < MAX_EVENTS_HANDLE_PER_CALL; i++)
	{
		if (eventsHandle[i] == h)
		{
			u32 nToCopy = MAX_EVENTS_HANDLE_PER_CALL - (i+1);
			if (nToCopy)
				memcpy(&eventsHandle[i], &eventsHandle[i + 1], sizeof(HANDLE) * nToCopy);
			eventsHandle[MAX_EVENTS_HANDLE_PER_CALL-1] = INVALID_HANDLE_VALUE;
			return;
		}
	}
	*/
}

//***********************************************
void OSWaitableGrp::removeSocket (OSSocket &sok)
{ 
	assert (debug_bWaiting == 0);
	rhea::Allocator *allocator = rhea::memory_getSysHeapAllocator();

	sRecord *q = NULL;
	sRecord *p = base;
	while (p)
	{
		if (p->originType == eEventOrigin::socket)
		{

			if (platform::socket_compare(sok, p->origin.osSocket.sok))
			{
				priv_removeHandle (p->origin.osSocket.hEventNotify);
				WSACloseEvent(p->origin.osSocket.hEventNotify);

				
				//rimuovo eventuali eventi che sono ancora nel miobuffer di eventi-generati
				for (u32 i = 0; i < nEventsReady; i++)
				{
					if (generatedEventList[i]->originType == eEventOrigin::socket)
					{
						if (platform::socket_compare (sok, generatedEventList[i]->origin.osSocket.sok))
						{
							generatedEventList[i]->originType = eEventOrigin::deleted;
						}
					}
				}

				if (q == NULL)
					base = base->next;
				else
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
	assert(debug_bWaiting == 0);
	sRecord *s = priv_newRecord();
	s->originType = eEventOrigin::socket;
	s->origin.osSocket.sok = sok;
	s->origin.osSocket.hEventNotify = WSACreateEvent();

	//WSAEventSelect(sok.socketID, sok.hEventNotify, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE);
	WSAEventSelect (sok.socketID, s->origin.osSocket.hEventNotify, FD_READ | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_QOS | FD_ROUTING_INTERFACE_CHANGE | FD_ADDRESS_LIST_CHANGE);

    return s;
}


//***********************************************
void OSWaitableGrp::removeEvent (const OSEvent &evt)
{
	assert(debug_bWaiting == 0);
	rhea::Allocator *allocator = rhea::memory_getSysHeapAllocator();

	sRecord *q = NULL;
	sRecord *p = base;
	while (p)
	{
		if (p->originType == eEventOrigin::osevent)
		{
			if (platform::event_compare(p->origin.osEvent.evt, evt))
			{
				priv_removeHandle(p->origin.osEvent.evt.h);

				//rimuovo eventuali eventi che sono ancora nel miobuffer di eventi-generati
				for (u32 i = 0; i < nEventsReady; i++)
				{
					if (generatedEventList[i]->originType == eEventOrigin::osevent)
					{
						if (platform::event_compare(evt, generatedEventList[i]->origin.osEvent.evt))
						{
							generatedEventList[i]->originType = eEventOrigin::deleted;
						}
					}
				}

				if (q == NULL)
					base = base->next;
				else
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
	assert(debug_bWaiting == 0);
	sRecord *s = priv_newRecord();
	s->originType = eEventOrigin::osevent;
    s->origin.osEvent.evt = evt;
    return s;
}

//***********************************************
u8 OSWaitableGrp::wait(u32 timeoutMSec)
{
	assert(debug_bWaiting == 0);
	debug_bWaiting = 1;
	u8 ret = priv_wait(timeoutMSec);
	debug_bWaiting = 0;
	return ret;
}

//***********************************************
u8 OSWaitableGrp::priv_wait(u32 timeoutMSec)
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


	for (int i = 0; i < MAX_EVENTS_HANDLE_PER_CALL; i++)
		eventsHandle[i] = INVALID_HANDLE_VALUE;

	//creo la lista di handle sulla quale fare la WaitForMultipleObjects
	DWORD n = 0;
	sRecord *p = base;
	while (p)
	{
		switch (p->originType)
		{
		default:
			DBGBREAK;
			break;

		case eEventOrigin::socket:
			eventsHandle[n++] = p->origin.osSocket.hEventNotify;
			break;

		case eEventOrigin::osevent:
			eventsHandle[n++] = p->origin.osEvent.evt.h;
			break;
		}
		p = p->next;
	}

	if (n > MAX_EVENTS_HANDLE_PER_CALL)
	{
		//posso gestire un numero finito di handle.
		//Eventuali altri handle semplicemente li ignoro
		n = MAX_EVENTS_HANDLE_PER_CALL;
	}
	
	//timeout per la wait
	DWORD dwMilliseconds = (DWORD)timeoutMSec;
	if (timeoutMSec == u32MAX)
		dwMilliseconds = INFINITE;

	if (n == 0)
	{
		assert (dwMilliseconds != INFINITE);
		::Sleep(dwMilliseconds);
		return 0;
	}



	DEBUG_PRINTF("\n\nOSWaitableGrp::wait\n");
	DEBUG_PRINTF("  n=%d\n", n);
	for (int i = 0; i < MAX_EVENTS_HANDLE_PER_CALL; i++)
		DEBUG_PRINTF("  %X", eventsHandle[i]);

	//Attendo pazientemente...
	nEventsReady = 0;
	assert(debug_bWaiting == 1);
	DWORD ret = WaitForMultipleObjects(n, eventsHandle, FALSE, dwMilliseconds);
	assert(debug_bWaiting == 1);

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
		

	//cerco l'handle che ha generato l'interruzione
	if (index < MAX_EVENTS_HANDLE_PER_CALL)
	{
		p = base;
		while (p)
		{
			bool bFound = false;
			switch (p->originType)
			{
			default:
				DBGBREAK;
				break;

			case eEventOrigin::socket:
				if (eventsHandle[index] == p->origin.osSocket.hEventNotify)
					bFound = true;
				break;

			case eEventOrigin::osevent:
				if (eventsHandle[index] == p->origin.osEvent.evt.h)
					bFound = true;
				break;
			}

			if (bFound)
			{
				if (p->originType == eEventOrigin::socket)
				{
					WSANETWORKEVENTS networkEvents;
					if (0 != WSAEnumNetworkEvents(p->origin.osSocket.sok.socketID, p->origin.osSocket.hEventNotify, &networkEvents))
					{
						int errCode = WSAGetLastError();
						switch (errCode)
						{
						case WSANOTINITIALISED:
							//A successful WSAStartup call must occur before using this function.
							DBGBREAK;
							break;
						case WSAENETDOWN:
							//The network subsystem has failed.
							DBGBREAK;
							break;
						case WSAEINVAL:
							//One of the specified parameters was invalid.
							DBGBREAK;
							break;
						case WSAEINPROGRESS:
							//A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
							DBGBREAK;
							break;
						case WSAENOTSOCK:
							//The descriptor is not a socket.
							DBGBREAK;
							break;
						case WSAEFAULT:
							//The lpNetworkEvents parameter is not a valid part of the user address space.
							DBGBREAK;
							break;

						}
					}

					//if (networkEvents.lNetworkEvents == 0) ::ResetEvent(p->origin.osSocket.hEventNotify);

					DEBUG_PRINTF("  was a socket [userparam=%d], event bits = 0x%X\n", p->userParam.asU32, networkEvents.lNetworkEvents);
					if ((networkEvents.lNetworkEvents & FD_CLOSE) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_CLOSE")
					if ((networkEvents.lNetworkEvents & FD_READ) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_READ")
					if ((networkEvents.lNetworkEvents & FD_WRITE) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_WRITE")
					if ((networkEvents.lNetworkEvents & FD_OOB) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_OOB")
					if ((networkEvents.lNetworkEvents & FD_ACCEPT) != 0)					ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ACCEPT")
					if ((networkEvents.lNetworkEvents & FD_CONNECT) != 0)					ADD_EVENT_AND_DEBUG_TEXT(p, "FD_CONNECT")
					if ((networkEvents.lNetworkEvents & FD_QOS) != 0)						ADD_EVENT_AND_DEBUG_TEXT(p, "FD_QOS")
					if ((networkEvents.lNetworkEvents & FD_ROUTING_INTERFACE_CHANGE) != 0)	ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ROUTING_INTERFACE_CHANGE")
					if ((networkEvents.lNetworkEvents & FD_ADDRESS_LIST_CHANGE) != 0)		ADD_EVENT_AND_DEBUG_TEXT(p, "FD_ADDRESS_LIST_CHANGE")
				}
				else if (p->originType == eEventOrigin::osevent)
				{
					generatedEventList[nEventsReady++] = p;
				}

				return nEventsReady;
			}
			p = p->next;
		}
	}

	DEBUG_PRINTF("  handle not found\n", ret);

	return 0;

#undef DEBUG_PRINTF
#undef ADD_EVENT_AND_DEBUG_TEXT
}

//***********************************************
OSWaitableGrp::eEventOrigin OSWaitableGrp::getEventOrigin (u8 iEvent) const
{
	assert(debug_bWaiting == 0);
    assert (iEvent < nEventsReady);
	return generatedEventList[iEvent]->originType;
}

//***********************************************
void* OSWaitableGrp::getEventUserParamAsPtr (u8 iEvent) const
{
	assert(debug_bWaiting == 0);
	assert(iEvent < nEventsReady);
	return generatedEventList[iEvent]->userParam.asPtr;
}

//***********************************************
u32 OSWaitableGrp::getEventUserParamAsU32 (u8 iEvent) const
{
	assert(debug_bWaiting == 0);
	assert(iEvent < nEventsReady);
	return generatedEventList[iEvent]->userParam.asU32;
}

//***********************************************
OSSocket& OSWaitableGrp::getEventSrcAsOSSocket (u8 iEvent) const
{
	assert(debug_bWaiting == 0);
    assert (getEventOrigin(iEvent) == eEventOrigin::socket);
    return generatedEventList[iEvent]->origin.osSocket.sok;
}

//***********************************************
OSEvent& OSWaitableGrp::getEventSrcAsOSEvent (u8 iEvent) const
{
	assert(debug_bWaiting == 0);
    assert (getEventOrigin(iEvent) == eEventOrigin::osevent);
	return generatedEventList[iEvent]->origin.osEvent.evt;
}

/***********************************************
OSSerialPort& OSWaitableGrp::getEventSrcAsOSSerialPort (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == eEventOrigin::serialPort);
    return currentEvent->origin.osSerialPort;
}
*/
#endif