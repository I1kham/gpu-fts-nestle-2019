#ifndef _rheaLinearBuffer_h_
#define _rheaLinearBuffer_h_
#include "rheaMemory.h"


namespace rhea
{
    /******************************************************************************
     * LinearBuffer
     *
     * Gestisce un blocco di memoria, che viene mantenuto contiguo anche a seguito di eventuale crescita del buffer.
     * Nel momento in cui si volesse far crescere il buffer, LinearBuffer si appoggia ad un Allocator per riservare nuova
     * memoria. Il buffer precedente viene memcpy nel buffer nuovo.
     * In questo modo il blocco di memoria è sempre contiguo, anche a seguito di espansione
    *******************************************************************************/
    class LinearBuffer
    {
    public:
                        LinearBuffer();
                        ~LinearBuffer();

        void			setupWithBase (void *startingBlock, u32 sizeOfStartingBlock, rhea::Allocator *backingallocator);
                            /*	utilizza startingBlock come blocco di memoria iniziale. Non ne fa il free.
                                Se il buffer dovesse espandersi, allora nuova memoria viene allocata tramite il backingallocator
                                e il contenuto di startingBlock viene memcpiato nel nuovo buffer.
                                In ogni caso, startingBlock non viene mai freed, anche perchè non è possibile sapere da chi è stato allocato
                            */

        void			setup (rhea::Allocator *backingallocator, u32 preallocNumBytes=0);
                            /*	alloca un blocco iniziale di dimensione=preallocNumBytes usando il backingallocator.
                                Se il buffer dovessere crescere, nuova memoria viene allocata tramite il backingallocator e il contenuto
                                del precedente buffer viene memcpiato nel nuovo.
                                Il buffer precedente viene poi freed()
                            */

        void			unsetup ()																								{ priv_FreeCurBuffer(); mem = NULL; allocatedSize = 0; bFreeMemBlock = 0; allocator = NULL; }

        bool			copyFrom (const LinearBuffer &src, u32 srcOffset, u32 nBytesToCopy, u32 dstOffset, bool bCangrow=true);
                            /* copia [nBytesToCopy] bytes di [src] a partire da [srcOffset] e li mette in this a partire da [dstOffset].
                                Valgono le stesse considerazioni di read/write relativamente al fallimento della funzione
                            */

        void			shrink (u32 newSize, rhea::Allocator *newAllocator = NULL);
                            /*	riduce la dimensione del buffer allocato portandola a newSize e memcpyando tutti i dati fino a newSize.
                                se viene indicato un newAllocator, allora si usa quello, altrimenti si usa il solito allocator
                            */

        u8*				_getPointer (u32 pos) const																				{ assert(pos<allocatedSize); return &mem[pos]; }

        bool			read  (void *dest, u32 offset, u32 nBytesToread) const;
        bool			write (const void *src, u32 offset, u32 nBytesTowrite, bool bCangrow=true);
        bool			growIncremental (u32 howManyBytesToAdd);
        bool			growUpTo (u32 finalSize);
        u32				getTotalSizeAllocated() const																			{ return allocatedSize; }

        rhea::Allocator*	getAllocator() const                                                                                { return allocator; }

    private:
                        RHEA_NO_COPY_NO_ASSIGN(LinearBuffer);
        void			priv_FreeCurBuffer ();

    private:
        rhea::Allocator	*allocator;
        u8				*mem;
        u32				allocatedSize;
        u8				bFreeMemBlock;
    };

}; //namespace rhea
#endif // _rheaLinearBuffer_h_
