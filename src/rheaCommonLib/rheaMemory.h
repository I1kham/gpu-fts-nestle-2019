#ifndef _rheaMemory_h_
#define _rheaMemory_h_
#include "rheaDataTypes.h"
#include <string.h> // per memcpy
#include <new> //per placement new
#include "rheaAllocator.h"



#define RHEANEW(allocator, T)                               new ( (*allocator).alloc( sizeof(T), __alignof(T) )) T

        template <class T>
void	RHEADELETE(rhea::Allocator *alloc, T* &p)			{ if (p) { p->~T(); (*alloc).dealloc(p); p=NULL; } }

#define RHEAALLOCSTRUCT(allocator,T)                        (T*)(allocator)->alloc( sizeof(T), __alignof(T) );


#define RHEA_NO_COPY_NO_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

namespace rhea
{
    bool            internal_memory_init();     //uso interno (usate da rhea::init())
    void            internal_memory_deinit();   //uso interno (usate da rhea::deinit())

    Allocator*      memory_getDefaultAllocator();

    size_t          memory_getSizeOfAPointer();

} // namespace rhea




#endif // _rheaMemory_h_

