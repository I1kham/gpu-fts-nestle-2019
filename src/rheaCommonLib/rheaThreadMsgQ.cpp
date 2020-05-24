#include "rheaThread.h"
#include "rheaFIFO.h"
#include "rheaUtils.h"

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
    glob.allocator = rhea::getSysHeapAllocator();
    glob.handleArray.setup (glob.allocator, 1024);
    rhea::criticalsection::init (&glob.cs);

    return true;
}


//**************************************************************
void thread::internal_deinit()
{
    glob.handleArray.unsetup();
    rhea::criticalsection::close(glob.cs);
}

//**************************************************************
sThreadMsgQ* thread_fromHandleToPointer (const HThreadMsgR &h)
{
    sThreadMsgQ *s = NULL;
    //rhea::criticalsection::enter(glob.cs);
        if (glob.handleArray.fromHandleToPointer(h, &s))
        {
            if (s->handle != h)
                s = NULL;
        }
    //rhea::criticalsection::leave(glob.cs);


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
    rhea::criticalsection::enter(glob.cs);
    {
        s = glob.handleArray.alloc();
    }
    rhea::criticalsection::leave(glob.cs);

    if (NULL == s)
    {
        DBGBREAK;
        return false;
    }

    s->fifo = RHEANEW(glob.allocator, FIFO<thread::sMsg>) ();
	s->fifo->setup(glob.allocator);
	rhea::event::open(&s->osEvent);
    rhea::criticalsection::init (&s->cs);

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

    rhea::criticalsection::enter(s->cs);
    {
        thread::sMsg msg;
        while ( s->fifo->pop(&msg) )
            thread::deleteMsg (msg);

		s->fifo->unsetup();
        RHEADELETE( glob.allocator, s->fifo );
    }
    rhea::criticalsection::leave(s->cs);
    rhea::criticalsection::close(s->cs);

	rhea::event::close (s->osEvent);


    rhea::criticalsection::enter(glob.cs);
    {
        glob.handleArray.dealloc (handleR);
    }
    rhea::criticalsection::leave(glob.cs);
}

//**************************************************************
u32 thread::serializeMsg (const sMsg &msg, u8 *out_buffer, u32 sizeof_out_buffer)
{
    const u32 bytesNeeded = 2 + sizeof(msg.what) + sizeof(msg.paramU32) + sizeof(msg.bufferSize) + msg.bufferSize;
    assert (bytesNeeded < 0xffff);
    if (sizeof_out_buffer < bytesNeeded)
    {
        DBGBREAK;
        return 0;
    }
    u32 ct = 0;
    rhea::utils::bufferWriteU16 (&out_buffer[ct], (u16)bytesNeeded);
    ct += 2;

    rhea::utils::bufferWriteU16 (&out_buffer[ct], msg.what);
    ct += 2;

    rhea::utils::bufferWriteU32 (&out_buffer[ct], msg.paramU32);
    ct += 4;

    rhea::utils::bufferWriteU32 (&out_buffer[ct], msg.bufferSize);
    ct += 4;

    if (msg.bufferSize)
    {
        memcpy (&out_buffer[ct], msg.buffer, msg.bufferSize);
        ct += msg.bufferSize;
    }
    return ct;
}

//**************************************************************
u32 thread::deserializMsg (const u8 *buffer, u32 nBytesToUse, u16 *out_what, u32 *out_paramU32, u32 *out_bufferSize, const u8 **out_bufferPt)
{
    u32 ct = 0;
    const u16 bytesNeeded = rhea::utils::bufferReadU16 (&buffer[ct]);
    ct += 2;

    *out_what = rhea::utils::bufferReadU16 (&buffer[ct]);
    ct += 2;

    *out_paramU32 = rhea::utils::bufferReadU32 (&buffer[ct]);
    ct += 4;

    *out_bufferSize = rhea::utils::bufferReadU32 (&buffer[ct]);
    ct += 4;

    if (0 == *out_bufferSize)
        *out_bufferPt = NULL;
    else
    {
        *out_bufferPt = &buffer[ct];
        ct += *out_bufferSize;
    }

    return ct;
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

    rhea::criticalsection::enter(s->cs);
        s->fifo->push(msg);
		rhea::event::fire(s->osEvent);
    rhea::criticalsection::leave(s->cs);
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

    rhea::criticalsection::enter(s->cs);
        bool ret = s->fifo->pop(out_msg);
    rhea::criticalsection::leave(s->cs);

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

