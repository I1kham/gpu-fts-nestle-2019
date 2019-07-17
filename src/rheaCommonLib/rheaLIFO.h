#ifndef _rheaLIFO_h_
#define _rheaLIFO_h_
#include "rheaThreadSafePolicy.h"

namespace rhea
{
    /***********************************************************************
     * template di un LIFO generica, con opzioni per la politica di thread safe
     * Questa LIFO "alloca" staticamente N elementi di tipo T, quindi non può crescere (ie: il numero massimo di elementi memorizzabili è fissato a priori).
     * Il tipo T deve essere o un tipo semplice (int, float..) oppure una POD, ovvero deve essere possibile memcpiarlo.
     *
     * Vedi le classi LIFOFixed e LIFOFixedTs a fine file per la versione non threadsafe e per la versione threadsafe
     * di questa classe
     */
    template <typename T, int N, typename TThreadSafePolicy>
    class TemplateLIFOFixed
    {
    public:
                            TemplateLIFOFixed ()                            { nElem = 0; }
        virtual             ~TemplateLIFOFixed()                            { }

        void				empty()											{ tsPolicy.lock(); nElem=0; tsPolicy.unlock(); }

        bool				push (const T &data)
                            {
                                tsPolicy.lock();
                                assert (nElem < N-1);

                                if (nElem >= N)
                                {
                                    tsPolicy.unlock();
                                    return false;
                                }

                                memcpy (&blob[nElem++], &data, sizeof(T));
                                tsPolicy.unlock();
                                return true;
                            }

        bool				pop (T *out_data)
                            {
                                tsPolicy.lock();
                                if (nElem == 0)
                                {
                                    tsPolicy.unlock();
                                    return false;
                                }

                                --nElem;
                                memcpy (out_data, &blob[nElem], sizeof(T));
                                tsPolicy.unlock();
                                return true;
                            }

        bool				top (T *out_data) const
                            {
                                tsPolicy.lock();
                                if (nElem == 0)
                                {
                                    tsPolicy.unlock();
                                    return false;
                                }

                                memcpy (out_data, &blob[nElem-1], sizeof(T));
                                tsPolicy.unlock();
                                return true;
                            }
    private:
        TThreadSafePolicy   tsPolicy;
        u32					nElem;
        T					blob[N];
    };


    /***********************************************************************
     * LIFO fixed non thread safe
     *
     */
    template<typename T, int N>
    class LIFOFixed : public TemplateLIFOFixed<T, N, ThreadSafePolicy_none>
    {
    public:
                    LIFOFixed ()        { }
        virtual     ~LIFOFixed ()       { }
    };




    /***********************************************************************
     * LIFO fixed thread safe
     *
     */
    template<typename T, int N>
    class LIFOFixedTs : public TemplateLIFOFixed<T, N, ThreadSafePolicy_cs>
    {
    public:
                    LIFOFixedTs ()          { }
        virtual     ~LIFOFixedTs ()         { }
    };
} // namespace rhea
#endif // _rheaLIFO_h_

