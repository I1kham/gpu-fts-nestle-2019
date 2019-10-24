#ifndef _rheaAllocator_h_
#define _rheaAllocator_h_
#include "rheaDataTypes.h"
#include "rheaString.h"
#include <string.h>

//macro di comodo per allineare "num" alla potenza del num "align" più vicina (più grande)
#ifndef ALIGN_NUMEBER_TO_POWER_OF_TWO
	#define ALIGN_NUMEBER_TO_POWER_OF_TWO(num, align) \
					(((num) + ((align) - 1)) & ~((align) - 1))
#endif

//macro di comodo, ritorna true se "pointer" è allineato alla potenza del 2 "align"
#ifndef IS_POINTER_ALIGNED
	#define IS_POINTER_ALIGNED(pointer, align) \
			(((uintptr_t)(const void *)(pointer)) % (align) == 0)
#endif

namespace rhea
{

    /*******************************************************************************************
     * Allocator
     *
     *
     *******************************************************************************************/
    class Allocator
    {
    public:
							Allocator(const char *nameIN)													{ myID = allocatorID++; rhea::string::copy_s(name, sizeof(name), nameIN); }
        virtual             ~Allocator()																	{ }


							//[bPlacementNew] è più che altro per questioni di debug e serve ad indicare se la alloc si sta
							//utilzzando per instanziare una classe (true) oppure se è una plain malloc (false).
							//Così facendo, durante la dealloc() posso accertarmi che le cose allocate con RHEANEW siano deallocate con RHEADELETE e tutto il resto
							//invece con RHEAFREE
#ifdef _DEBUG
		void*               alloc (size_t sizeInBytes, size_t align, const char *debug_filename, u32 debug_lineNumber, bool bPlacementNew = false)
							{
								void *ret = virt_do_alloc(sizeInBytes, align, debug_filename, debug_lineNumber, bPlacementNew);
								if (ret)
									memset(ret, 0xCA, sizeInBytes);
								return ret;
							}
#else
		void*               alloc (size_t sizeInBytes, size_t align, bool bPlacementNew = false)
							{
								return virt_do_alloc(sizeInBytes, align, "", 0, bPlacementNew);
							}									
#endif


        void                dealloc (void *p, bool bPlacementNew = false)									{ virt_do_dealloc(p, bPlacementNew); }

        const char*         getName() const																	{ return name; }
		u16					getAllocatorID() const															{ return myID; }

        virtual bool        isThreadSafe() const = 0;
        virtual size_t      getAllocatedSize (const void *p) const = 0;

    protected:
        virtual void*       virt_do_alloc (size_t sizeInBytes, size_t align, const char *debug_filename, u32 debug_lineNumber, bool bPlacementNew) = 0;
        virtual void        virt_do_dealloc (void *p, bool bPlacementNew) = 0;

	private:
		static u16			allocatorID;

    private:
        char                name[16];
		u16					myID;
    };
}

#endif // _rheaAllocator_h_
