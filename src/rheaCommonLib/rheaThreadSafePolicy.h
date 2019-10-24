#ifndef _rheaThreadSafePolicy_h_
#define _rheaThreadSafePolicy_h_
#include "OS/OS.h"

namespace rhea
{
    /***********************************************************************
    * ThreadSafePolicy_none
    *
    * Non implementa alcun meccanismo di lock (quindi non è thread safe)
    *
    ***********************************************************************/
    class ThreadSafePolicy_none
    {
    public:
                        ThreadSafePolicy_none ()                        { }
                        ~ThreadSafePolicy_none ()                       { }

        void			lock ()	const									{ }
        void			unlock () const									{ }
        bool            isThreadSafe() const                            { return false; }

    private:
                        RHEA_NO_COPY_NO_ASSIGN(ThreadSafePolicy_none);
    };

    /***********************************************************************
    * ThreadSafePolicy_cs
    *
    * Implementa il meccanismo di lock usando una Critical Section
    *
    ***********************************************************************/
    class ThreadSafePolicy_cs
    {
    public:
                        ThreadSafePolicy_cs ()                          { rhea::criticalsection::init(&cs); }
                        ~ThreadSafePolicy_cs ()							{ rhea::criticalsection::close(cs); }

        void			lock ()	const									{ rhea::criticalsection::enter(cs); }
        void			unlock () const									{ rhea::criticalsection::leave(cs); }
        bool            isThreadSafe() const                            { return true; }

    private:
                        RHEA_NO_COPY_NO_ASSIGN(ThreadSafePolicy_cs);

    private:
        mutable OSCriticalSection	cs;
    };

} // namespace rhea


#endif // _rheaThreadSafePolicy_h_

