#ifndef _rheaHandleBaseArray_h_
#define _rheaHandleBaseArray_h_
#include "rheaDataTypes.h"
#include "rheaMemory.h"


namespace rhea
{
    /*=================================================================================
     * BaseHandleArray
     *
     * Template di base per i 2 template MTHandleArray e HandleArray
     *
     * Per rendere questa classe thread safe, bisogna usare MTIntrusiveFreeList e un Allocator thread safe (vedi GOSMTHandleArray.h)
     * Per rendere questa classe NON thread safe, bisogna usare IntrusiveFreeList e un Allocator non thread safe (vedi GOSHandleArray.h)
     * Alloca un buffer di memoria usando [Allocator]. Il buffer è grosso [numMaxItem] * [sizeOfAnItem]
     * Ad ogni new() viene restituito un [T] preso dal buffer. Ad ogni T viene associato un handle univoco che lo identifica
     *
     * Il tipo T deve avere per forza un membro "THandle handle", posizionato almeno 4 byte dopo l'inizio di T
     * Il membro THandle va definito con la macro RHEATYPEDEF_HANDLExxxx(nomeDelTipo)
     *
     * Ad es:
     *	RHEATYPEDEF_HANDLE1616(MyHandle);
     *	struct sTipo
     *	{
     *		u32			whatever;
            MyHandle	handle;
            ....
        };
     *
     *  T deve avere 3 funzioni:
     *		oneTimeInit()	->	chiamata durante la setup() e serve per inizializzare eventuali membri di T
     *		oneTimeDeinit()	->	chiamata durante la unsetup
     *		onAlloc()       ->	chiamata ogni volta che si crea un nuovo oggett usando la BaseHandleArray->alloc()
     *      onDealloc()     ->	chiamata ogni volta che si crea un nuovo oggett usando la BaseHandleArray->dealloc()
     *=================================================================================*/
    template<class TFreeList, typename T, typename THandle>
    class BaseHandleArray
    {
    public:
                                        //***********************************************************
                                        BaseHandleArray ()
                                        {
                                            allocator = NULL;

                                            //devo assicurarmi che nella struttura, l'handle sia almeno 4 byte dopo l'inizio, visto che i primi 4 byte sono manipolati da freeList
                                            T	test;

                                            if (PTR_DIFF_TO_INT(&test.handle, &test) < 4)
                                                rhea::logger->logErr ("BaseHandleArray<T>::BaseHandleArray() -> Type does not match specifications") << rhea::Logger::EOL;

                                            //mi assicuro che il parametro handle sia = al tipo THandle indicato nel template
#ifdef _DEBUG
                                            THandle testTHandle;
                                            testTHandle.init(1,3);
                                            test.handle = testTHandle;
#endif
                                        }

                                        //***********************************************************
        virtual							~BaseHandleArray()																					{ unsetup(); }

                                        //***********************************************************
        bool							setup (rhea::Allocator *allocatorIN, u32 numMaxItem, u32 sizeOfAnItem = sizeof(T), u32 indexOfFirstItemIN=0)
                                        {
                                            assert (numMaxItem < 0xFFFF);
                                            assert (sizeof(T) <= sizeOfAnItem);
                                            assert (allocatorIN);
                                            //assert (indexOfFirstItemIN < 0xFFFF &&  indexOfFirstItemIN < 0xFFFF - numMaxItem);

                                            allocator = allocatorIN;
                                            indexOfFirstItem = indexOfFirstItemIN;
                                            const u32 sizeOfMemBlock = sizeOfAnItem * numMaxItem;
                                            memblock = (u8*)RHEAALLOC(allocator, sizeOfMemBlock);
                                            assert (NULL != memblock);
                                            memset (memblock, 0, sizeOfMemBlock);

                                            freelist.setup (memblock, sizeOfMemBlock, sizeOfAnItem, numMaxItem);

                                            //imposto i valori iniziali degli handle
                                            u32	offset=0;
                                            for (u32 i=0; i<numMaxItem; i++)
                                            {
                                                T *j = (T*)&memblock[offset];
                                                j->handle.init (indexOfFirstItem + i, 1);
                                                j->oneTimeInit();
                                                offset += sizeOfAnItem;
                                            }
                                            return true;
                                        }

                                        //***********************************************************
        void							fast_unsetup ()
                                        {
                                            if (allocator)
                                            {
                                                allocator->dealloc (memblock);
                                                allocator = NULL;
                                            }
                                        }

        void							unsetup ()
                                        {
                                            if (allocator)
                                            {
                                                u32 n=0;
                                                T *j;
                                                while ((j=freelist.alloc()))
                                                {
                                                    j->oneTimeDeinit();
                                                    ++n;
                                                }
                                                assert (n == freelist.getNumMaxItem());
                                                allocator->dealloc (memblock);
                                                allocator = NULL;
                                            }
                                        }

                                        //***********************************************************
        T*								allocIfYouCan()
                                        {
                                            T *j = freelist.alloc();
                                            if (NULL == j)
                                                return NULL;
                                            assert (j->handle.getIndex() >= indexOfFirstItem);
                                            assert (j->handle.getIndex() - indexOfFirstItem == freelist.getIndexFromPointer(j));
                                            j->onAlloc();
                                            return j;
                                        }

        T*								alloc()
                                        {
                                            T *j = allocIfYouCan();
                                            assert (j);
                                            return j;
                                        }

                                        //***********************************************************
        void							dealloc (THandle &h)
                                        {
                                            T *t;
                                            if (!fromHandleToPointer (h, &t))
                                            {
                                                h.setInvalid();
                                                return;
                                            }

                                            //se l'h che ho passato come parametro a questa fn è lo stesso h della struttura, non va bene
                                            //perchè io voglio invalidare l'h passatomi ma cosi' facendo, invaliderei anche l'h della struttura
                                            if (&t->handle != &h)
                                                h.setInvalid();
                                            t->handle.incCounter();
                                            t->onDealloc();
                                            freelist.dealloc (t);
                                        }

                                        //***********************************************************
        bool							isValidHandle (const THandle &h) const																{ T *t; return fromHandleToPointer (h, &t); }

                                        //***********************************************************
        bool							fromHandleToPointer (const THandle &h, T* *out) const
                                        {
                                            if (h.isInvalid())
                                                return false;
                                            u32 index = h.getIndex();
                                            assert (index >= indexOfFirstItem);
                                            index -= indexOfFirstItem;

                                            if (index >= freelist.getNumMaxItem())
                                            {
                                                *out = NULL;
                                                return false;
                                            }

                                            *out = freelist.getPointerFromIndex (index);
                                            assert(out);
                                            return ((*out)->handle == h);
                                        }

                                        //***********************************************************
        u32								getNAllocated() const																				{ return freelist.getNAllocated(); }
        rhea::Allocator*                getAllocator() const																				{ return allocator; }

    private:
                                        RHEA_NO_COPY_NO_ASSIGN(BaseHandleArray);

    private:
        rhea::Allocator					*allocator;
        TFreeList						freelist;
        u32								indexOfFirstItem;
        u8								*memblock;

    };
};

#endif // _rheaHandleBaseArray_h_

