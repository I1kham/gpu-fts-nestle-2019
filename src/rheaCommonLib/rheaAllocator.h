#ifndef _rheaAllocator_h_
#define _rheaAllocator_h_
#include "rheaDataTypes.h"
#include "rheaString.h"
#include <string.h>

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
                            Allocator(const char *nameIN)                       { rhea::string::copy_s (name, sizeof(name), nameIN); }
        virtual             ~Allocator()                                        { }

                            //questa usa l'allineamento di default (come fa la malloc)
        void*               alloc (size_t sizeInBytes)                          { return alloc(sizeInBytes, 0); }

                            //questa permette di indicare un allineamento
        void*               alloc (size_t sizeInBytes, size_t align)
                            {
                                #ifdef _DEBUG
                                    void *ret = priv_do_alloc(sizeInBytes, align);
                                    if (ret)
                                        memset (ret, 0xCA, sizeInBytes);
                                    return ret;
                                #else
									return priv_do_alloc(sizeInBytes, align);
                                #endif
                            }


        void                dealloc (const void *p)                             { priv_do_dealloc(p); }

        const char*         getName() const                                     { return name; }

        virtual bool        isThreadSafe() const = 0;

    protected:
        virtual void*       priv_do_alloc (size_t sizeInBytes, size_t align) = 0;
        virtual void        priv_do_dealloc (const void *p) = 0;

    private:
        char                name[16];
    };
}

#endif // _rheaAllocator_h_
