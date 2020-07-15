#ifndef _rheaMemory_h_
#define _rheaMemory_h_
#include "rheaDataTypes.h"
#include <string.h> // per memcpy
#include <new> //per placement new
#include "rheaAllocator.h"


/***********************************************************************
 * Macro per la gestione delle allocazioni dinamiche
 *
 */
#ifdef _DEBUG
	#define RHEANEW(allocator, T)                               new ( (*allocator).alloc( sizeof(T), __alignof(T), __FILE__, __LINE__, true)) T


    #define RHEAALLOCSTRUCT(allocator,T)                        (T*)(allocator)->alloc( sizeof(T), __alignof(T), __FILE__, __LINE__, false)

    #define RHEAALIGNEDALLOC(allocator,sizeInByte,align)		(allocator)->alloc ((sizeInByte), align, __FILE__, __LINE__, false)
    #define RHEAALLOC(allocator,sizeInByte)                     (allocator)->alloc ((sizeInByte), __alignof(void*), __FILE__, __LINE__, false)
    #define RHEAALLOCT(returnType, allocator,sizeInByte)        static_cast<returnType>((allocator)->alloc ((sizeInByte), __alignof(void*), __FILE__, __LINE__, false))
#else
	#define RHEANEW(allocator, T)                               new ( (*allocator).alloc( sizeof(T), __alignof(T), true)) T


    #define RHEAALLOCSTRUCT(allocator,T)                        (T*)(allocator)->alloc( sizeof(T), __alignof(T), false)

    #define RHEAALIGNEDALLOC(allocator,sizeInByte,align)		(allocator)->alloc ((sizeInByte), align, false);
    #define RHEAALLOC(allocator,sizeInByte)						(allocator)->alloc ((sizeInByte), __alignof(void*), false)
#endif


template <class T>
void	RHEADELETE(rhea::Allocator *alloc, T* &p) { if ((p)) { (p)->~T(); (*alloc).dealloc((p), true); (p) = NULL; } }

#define RHEAFREE(allocator,p)								(allocator)->dealloc ((p), false);





#define RHEA_NO_COPY_NO_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)



namespace rhea
{
    bool            internal_memory_init();     //uso interno (usate da rhea::init())
    void            internal_memory_deinit();   //uso interno (usate da rhea::deinit())
	
    Allocator*      memory_getSysHeapAllocator();
	Allocator*      memory_getScrapAllocator2();		//ritorna un allocatore specializzato per piccole e brevi (temporalmente parlando) allocazioni

    size_t          memory_getSizeOfAPointer();



	class MemoryTracker;
	MemoryTracker*	internal_getMemoryTracker();

} // namespace rhea




#endif // _rheaMemory_h_

