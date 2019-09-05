#include "rheaThread.h"
#include "rheaFIFO.h"

using namespace rhea;

#define HANDLE_AS_READ      1
#define HANDLE_AS_WRITE     2

struct sThreadMsgQ
{
    FIFO<thread::sMsg>      *fifo;
    OSEvent                 osEvent;
    OSCriticalSection       cs;
    HThreadMsgR             handle;

    void            oneTimeInit()	{}
    void            oneTimeDeinit()	{}
    void            onAlloc()       { fifo = NULL; }
    void            onDealloc()     {}
};


struct sGlob
{
    rhea::Allocator                             *allocator;
    rhea::HandleArray<sThreadMsgQ,HThreadMsgR>  handleArray;
    OSCriticalSection                           cs;
};
sGlob   glob;


//**************************************************************
bool thread::internal_init()
{
    glob.allocator = rhea::memory_getDefaultAllocator();
    glob.handleArray.setup (glob.allocator, 1024);
    OSCriticalSection_init (&glob.cs);

    return true;
}


//**************************************************************
void thread::internal_deinit()
{
    glob.handleArray.unsetup();
    OSCriticalSection_close(glob.cs);
}

//**************************************************************
sThreadMsgQ* thread_fromHandleToPointer (const HThreadMsgR &h)
{
    sThreadMsgQ *s = NULL;
    //OSCriticalSection_enter(glob.cs);
        if (glob.handleArray.fromHandleToPointer(h, &s))
        {
            if (s->handle != h)
                s = NULL;
        }
    //OSCriticalSection_leave(glob.cs);


    assert (s != NULL);

    return s;

}

//**************************************************************
sThreadMsgQ* thread_fromHandleToPointer (const HThreadMsgW &h)
{
    HThreadMsgR hRead;
    hRead.initFromU32 ( h.asU32() );
    hRead.setReserved ( HANDLE_AS_READ );
    return thread_fromHandleToPointer (hRead);
}

//**************************************************************
bool thread::createMsgQ (HThreadMsgR *out_handleR, HThreadMsgW *out_handleW)
{
    sThreadMsgQ *s = NULL;
    OSCriticalSection_enter(glob.cs);
    {
        s = glob.handleArray.alloc();
    }
    OSCriticalSection_leave(glob.cs);

    if (NULL == s)
    {
        DBGBREAK;
        return false;
    }

    s->fifo = RHEANEW(glob.allocator, FIFO<thread::sMsg>) (glob.allocator);
    OSEvent_open(&s->osEvent);
    OSCriticalSection_init (&s->cs);

    *out_handleR = s->handle;
    out_handleR->setReserved (HANDLE_AS_READ);    //lo marco come "read"

    out_handleW->initFromU32 (s->handle.asU32());
    out_handleW->setReserved (HANDLE_AS_WRITE);    //lo marco come "write"
    return true;
}

//**************************************************************
void thread::deleteMsgQ (HThreadMsgR &handleR, HThreadMsgW &handleW UNUSED_PARAM)
{
    sThreadMsgQ *s = thread_fromHandleToPointer(handleR);
    if (NULL == s)
        return;

    OSCriticalSection_enter(s->cs);
    {
        thread::sMsg msg;
        while ( s->fifo->pop(&msg) )
            thread::deleteMsg (msg);

        RHEADELETE( glob.allocator, s->fifo );
    }
    OSCriticalSection_leave(s->cs);
    OSCriticalSection_close(s->cs);

    OSEvent_close (s->osEvent);


    OSCriticalSection_enter(glob.cs);
    {
        glob.handleArray.dealloc (handleR);
    }
    OSCriticalSection_leave(glob.cs);
}

//**************************************************************
void thread::pushMsg (const HThreadMsgW &h, u16 what, u32 paramU32, const void *src, u32 sizeInBytes)
{
    sThreadMsgQ *s = thread_fromHandleToPointer(h);

    if (NULL == s)
        return;

    thread::sMsg msg;
    memset (&msg, 0x00, sizeof(msg));
    msg.what = what;
    msg.paramU32 = paramU32;
    if (src && sizeInBytes>0)
    {
        msg.bufferSize = sizeInBytes;
        msg.buffer = RHEAALLOC(glob.allocator, sizeInBytes);
        memcpy (msg.buffer, src, sizeInBytes);
    }
    else
    {
        msg.buffer = NULL;
        msg.bufferSize = 0;
    }

    OSCriticalSection_enter(s->cs);
        s->fifo->push(msg);
        OSEvent_fire(s->osEvent);
    OSCriticalSection_leave(s->cs);
}

//**************************************************************
bool thread::getMsgQEvent (const HThreadMsgR &h, OSEvent *out_hEvent)
{
    sThreadMsgQ *s = thread_fromHandleToPointer(h);

    if (NULL == s)
        return false;
    *out_hEvent = s->osEvent;
    return true;
}

//**************************************************************
bool thread::popMsg (const HThreadMsgR &h, thread::sMsg *out_msg)
{
    sThreadMsgQ *s = thread_fromHandleToPointer(h);
    if (NULL == s)
        return false;

    OSCriticalSection_enter(s->cs);
        bool ret = s->fifo->pop(out_msg);
    OSCriticalSection_leave(s->cs);

    return ret;
}

//**************************************************************
void thread::deleteMsg (const sMsg &msg)
{
    if (msg.bufferSize > 0 && msg.buffer != NULL)
    {
        glob.allocator->dealloc (msg.buffer);
    }
}





//**************************************************************
bool thread::create2WayMsgQ (sHThread2WayMsgQ *out)
{
	bool b = createMsgQ (&out->hRead[0], &out->hWrite[0]);
	if (!b)
		return false;

	b = createMsgQ (&out->hRead[1], &out->hWrite[1]);
	if (!b)
	{
		thread::deleteMsgQ(out->hRead[0], out->hWrite[0]);
		return false;
	}

	return true;
}

//**************************************************************
void thread::delete2WayMsgQ (sHThread2WayMsgQ &s)
{
	thread::deleteMsgQ (s.hRead[0], s.hWrite[0]);
	thread::deleteMsgQ (s.hRead[1], s.hWrite[1]);
}

//**************************************************************
bool thread::popMsg (const sHThread2WayMsgQ &h, e2WayMsgQThread me, sMsg *out_msg)
{
	return thread::popMsg (h.hRead[(u8)me], out_msg);
}

//**************************************************************
void thread::pushMsg (const sHThread2WayMsgQ &h, e2WayMsgQThread me, u16 what, u32 paramU32, const void *src, u32 sizeInBytes)
{
	const u8 i = 1 - (u8)me;
	thread::pushMsg (h.hWrite[i], what, paramU32, src, sizeInBytes);
}

