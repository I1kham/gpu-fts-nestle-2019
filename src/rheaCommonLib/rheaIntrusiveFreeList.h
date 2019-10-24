#ifndef _rheaIntrusiveFreeList_h_
#define _rheaIntrusiveFreeList_h_
#include "rheaDataTypes.h"


namespace rhea
{
    /*********************************************************************
    * IntrusiveFreeList
    *
    * FreeList intrusiva, NON thread safe
    * Questa classe gestisce un blocco di memoria preallocato da qualcun'altro, e lo tratta come un array di N oggetti di tipo T.
    * La dimensione di un singolo T è indicata da sizeOfAT.
    * Fornisce i metodi per l'allocazione e il rilascio di T
    * Per mantenere la lista dei free T, usa un u32 che posizione all'inizio di ogni T dell'array (in questo senso è intrusiva)
    *
    *	setup()					imposta il blocco iniziale di memoria da gestire
    *	alloc() / Free()		allocano/rilasciano blocchi
    *	getIndexFromPointer()	dato un T* ritornato da alloc(), questa fn ne ritorna il suo indice nell'array di memoria
    *	getPointerFromIndex()	dato un indice, ritorna il pt a T* dall'array di memoria
    *********************************************************************/
    template <class T>
    class IntrusiveFreeList
    {
    public:
                                IntrusiveFreeList ()
                                {
                                    _Init();
                                }

        void					_Init()
                                {
                                    blob = NULL;
                                    sizeOfAT = 0;
                                    N = 0;
                                    nallocated = 0;
                                }

        void					setup (void *memblock, u32 sizeOfMemBlock, u32 sizeOfAT, u32 N)
                                {
                                    assert (N < 0xFFFF);
                                    assert (sizeOfAT >= sizeof(u32));
                                    assert (sizeOfMemBlock >= N * sizeOfAT);
                                    assert (NULL == blob);

                                    this->sizeOfAT = sizeOfAT;
                                    this->N = N;
                                    nallocated = 0;
                                    blob = (u8*)memblock;

                                    freeIndex = 0;
                                    u32 offset = 0;
                                    for (u32 i=0; i<N-1; i++)
                                    {
                                        u32 *pNext = (u32*)&blob[offset];
                                        (*pNext) = i+1;
                                        offset += sizeOfAT;
                                    }
                                    u32 *pLast = (u32*)&blob[offset];
                                    (*pLast) = u32MAX;
                                }

        bool					isallocatedByMe (const T* t) const
                                {
                                    IntPointer t_addr = PTR_TO_INT(t);
                                    IntPointer blob_addr = PTR_TO_INT(blob);
                                    if (t_addr < blob_addr)
                                        return false;
                                    if((t_addr - blob_addr) / sizeOfAT >= N)
                                        return false;
                                    return true;
                                }

        u32						getIndexFromPointer(const T* t) const
                                {
                                    assert (isallocatedByMe(t));
                                    u32	tIndex = (u32)(PTR_DIFF_TO_INT(t,blob) / sizeOfAT);
                                    assert (tIndex < N);
                                    return tIndex;
                                }

        bool                    isValidIndex(u32 index) const                                                       { return (index < N); }
        T*						getPointerFromIndex (u32 index) const												{ assert (index < N); return (T*) &blob[index*sizeOfAT]; }

        T*						alloc ()
                                {
                                    if (freeIndex == u32MAX)
                                        return NULL;

                                    T *ret = getPointerFromIndex (freeIndex);
                                    freeIndex = ((u32*)ret)[0];
                                    ++nallocated;
                                    return ret;
                                }

        void					dealloc (T* t)
                                {
                                    u32	tIndex = getIndexFromPointer(t);
                                    u32	*pt	 = (u32*)t;
                                    *pt = freeIndex;
                                    freeIndex = tIndex;
                                    --nallocated;
                                }

        u32						getSizeOfAItem() const													{ return sizeOfAT; }
        u32						getNumMaxItem() const													{ return N; }
        u32						getNAllocated() const													{ return nallocated; }
        u8*						getMemBlock() const														{ return blob; }

    public:
                                RHEA_NO_COPY_NO_ASSIGN (IntrusiveFreeList);

    private:
        u32						freeIndex;
        u32						nallocated;
        u32						sizeOfAT;
        u32						N;
        u8						*blob;

    };
};


#endif // _rheaIntrusiveFreeList_h_

