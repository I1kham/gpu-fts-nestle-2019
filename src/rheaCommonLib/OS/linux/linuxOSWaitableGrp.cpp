#ifdef LINUX
#include <string.h>
#include "linuxOSWaitableGrp.h"
#include "rheaMemory.h"


//***********************************************
OSWaitableGrp::OSWaitableGrp()
{
    base = NULL;
    hfd = epoll_create1(0);
    assert (hfd!=-1);

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
    ::close(hfd);
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_newRecord (u32 flags)
{
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();
    sRecord *r = RHEAALLOCSTRUCT(allocator,sRecord);

    r->next = base;
    r->eventInfo.events = flags;
    r->eventInfo.data.ptr = r;

    base = r;
    return r;
}

//***********************************************
int OSWaitableGrp::priv_getFd (const sRecord *s) const
{
    switch (s->originType)
    {
    case evt_origin_socket:
        return s->origin.osSocket.socketID;

    case evt_origin_osevent:
        return s->origin.osEvent.evfd;

    default:
        DBGBREAK;
        return 0;
    }
}

//***********************************************
void OSWaitableGrp::priv_findAndRemoveRecordByFD (int fd)
{
    rhea::Allocator *allocator = rhea::memory_getDefaultAllocator();

    sRecord *q = NULL;
    sRecord *p = base;
    while (p)
    {
        if (priv_getFd(p) == fd)
        {
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

        q = p;
        p = p->next;
    }
}

//***********************************************
void OSWaitableGrp::priv_onRemove (int fd)
{
    priv_findAndRemoveRecordByFD(fd);

    /*  in teoria potrei passare NULL al posto di &eventInfo, ma pare ci sia un BUG in certe versioni di linux:
            In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required
            a non-null pointer in event, even though this argument is ignored.
    */
    epoll_event eventInfo;
    memset (&eventInfo, 0, sizeof(eventInfo));
    eventInfo.data.fd = fd;

    epoll_ctl(hfd, EPOLL_CTL_DEL, fd, &eventInfo);
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_addSocket (const OSSocket &sok)
{
    sRecord *s = priv_newRecord (EPOLLIN | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP);
    s->originType = (u8)evt_origin_socket;
    s->origin.osSocket = sok;

    int fd = sok.socketID;
    int err = epoll_ctl (hfd, EPOLL_CTL_ADD, fd, &s->eventInfo);
    if (err)
    {
        priv_findAndRemoveRecordByFD (fd);
        return NULL;
    }

    return s;
}


//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_addEvent (const OSEvent &evt)
{
    sRecord *s = priv_newRecord (EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP | EPOLLET);
    s->originType = (u8)evt_origin_osevent;
    s->origin.osEvent = evt;

    int fd = evt.evfd;
    int err = epoll_ctl (hfd, EPOLL_CTL_ADD, fd, &s->eventInfo);
    if (err)
    {
        priv_findAndRemoveRecordByFD(fd);
        return NULL;
    }


    return s;
}

//***********************************************
OSWaitableGrp::sRecord* OSWaitableGrp::priv_addSerialPort (const OSSerialPort &sp)
{
    sRecord *s = priv_newRecord (EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP);
    s->originType = (u8)evt_origin_serialPort;
    s->origin.osSerialPort = sp;

    int fd = sp.fd;
    int err = epoll_ctl (hfd, EPOLL_CTL_ADD, fd, &s->eventInfo);
    if (err)
    {
        priv_findAndRemoveRecordByFD(fd);
        return NULL;
    }


    return s;
}


//***********************************************
u8 OSWaitableGrp::wait(u32 timeoutMSec)
{
    int epollTimeout;
    if (timeoutMSec == u32MAX)
        epollTimeout = -1;
    if (timeoutMSec == 0)
        epollTimeout = 0;
    else
        epollTimeout = (int)timeoutMSec;

    nEventsReady = 0;
    int n = epoll_wait(hfd, events, MAX_EVENTS_PER_CALL, epollTimeout);
    if (n <= 0)
        return 0;

    nEventsReady = (u8)n;
    return nEventsReady;
}

//***********************************************
OSWaitableGrp::eEventOrigin OSWaitableGrp::getEventOrigin (u8 iEvent) const
{
    assert (iEvent < nEventsReady);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return (eEventOrigin)s->originType;
}

//***********************************************
void* OSWaitableGrp::getEventUserParamAsPtr (u8 iEvent) const
{
    assert (iEvent < nEventsReady);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return s->userParam.asPtr;
}

//***********************************************
u32 OSWaitableGrp::getEventUserParamAsU32 (u8 iEvent) const
{
    assert (iEvent < nEventsReady);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return s->userParam.asU32;
}

//***********************************************
OSSocket& OSWaitableGrp::getEventSrcAsOSSocket (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_socket);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return s->origin.osSocket;
}

//***********************************************
OSEvent& OSWaitableGrp::getEventSrcAsOSEvent (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_osevent);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return s->origin.osEvent;
}

//***********************************************
OSSerialPort& OSWaitableGrp::getEventSrcAsOSSerialPort (u8 iEvent) const
{
    assert (getEventOrigin(iEvent) == evt_origin_serialPort);

    sRecord *s = (sRecord*)events[iEvent].data.ptr;
    return s->origin.osSerialPort;
}

#endif
