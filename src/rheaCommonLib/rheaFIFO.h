#ifndef _rheaFIFO_h_
#define _rheaFIFO_h_
#include "rheaMemory.h"
#include "rheaThreadSafePolicy.h"

namespace rhea
{
    /***********************************************************************
     * template di un FIFO generica, con opzioni per la politica di thread safe
     * Il tipo T deve essere o un tipo semplice (int, float..) oppure una POD, ovvero deve essere possibile memcpiarlo.
     *
     * Vedi le classi FIFO e FIFOts a fine file per la versione non threadsafe e per la versione threadsafe
     * di questa classe
     */
    template<typename T, typename TThreadSafePolicy>
    class TemplateFIFO
    {
    public:
                    TemplateFIFO (Allocator *allocatorIN)
                    {
                        //se la policy è di thread-safe, è necessario che anche l'allocatore lo sia
                        assert( (tsPolicy.isThreadSafe() && allocatorIN->isThreadSafe()) || !tsPolicy.isThreadSafe() );
                        allocator=allocatorIN;
                        first = last = NULL;
                    }

        virtual     ~TemplateFIFO()                                                     { empty(); }

        void        empty()
                    {
                        tsPolicy.lock();
                            while (first)
                            {
                                sRecord *p = first;
                                first = first->next;
                                allocator->dealloc(p);
                            }
                            last = NULL;
                        tsPolicy.unlock();
                    }

        void        push (const T &data)
                    {
                        sRecord *p = (sRecord*)allocator->alloc(sizeof(sRecord));
                        memcpy (&p->data, &data, sizeof(T));
                        p->next = NULL;

                        tsPolicy.lock();
                            if (last)
                            {
                                last->next = p;
                                last = p;
                            }
                            else
                            {
                                first = last = p;
                            }
                        tsPolicy.unlock();
                    }

        bool        pop (T *out_data)
                    {
                        tsPolicy.lock();
                            if (NULL == first)
                            {
                                tsPolicy.unlock();
                                return false;
                            }

                            memcpy (out_data, &first->data, sizeof(T));
                            sRecord *p = first;
                            first = first->next;
                            allocator->dealloc(p);

                            if (NULL == first)
                                last = NULL;
                        tsPolicy.unlock();
                        return true;
                    }


    private:
        struct sRecord
        {
            sRecord *next;
            T       data;
        };

    private:
        Allocator           *allocator;
        sRecord             *first, *last;
        TThreadSafePolicy   tsPolicy;
    };




    /***********************************************************************
     * FIFO non thread safe
     *
     */
    template<typename T>
    class FIFO : public TemplateFIFO<T,ThreadSafePolicy_none>
    {
    public:
                    FIFO (Allocator *allocatorIN) : TemplateFIFO<T,ThreadSafePolicy_none>(allocatorIN)          { }
        virtual     ~FIFO ()                                                                                    { }
    };




    /***********************************************************************
     * FIFO thread safe
     *
     */
    template<typename T>
    class FIFOts : public TemplateFIFO<T,ThreadSafePolicy_cs>
    {
    public:
                    FIFOts (Allocator *allocatorIN) : TemplateFIFO<T,ThreadSafePolicy_cs>(allocatorIN)          { }
        virtual     ~FIFOts ()                                                                                  { }
    };

} //namespace rhea
#endif // _rheaFIFO_h_
