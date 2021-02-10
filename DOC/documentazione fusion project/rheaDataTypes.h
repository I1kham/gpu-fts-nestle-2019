#ifndef _rheaDataTypes_h_
#define _rheaDataTypes_h_
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

//typedef dei dati di base
typedef int8_t      i8;         //8 bit signed
typedef uint8_t     u8;         //8 bit unsigned
typedef int16_t     i16;        //16 bit signed
typedef uint16_t    u16;        //16 bit unsigned
typedef int32_t     i32;        //....
typedef uint32_t    u32;
typedef int64_t     i64;
typedef uint64_t    u64;
typedef float       f32;        //32 bit floating point

typedef uintptr_t   uiPtr;      //un "intero" la cui dimensione in byte dipende dalla piattaforma, ma che è sempre in grado di ospitare un puntatore


#define	u16MAX      0xFFFF
#define	u32MAX      0xFFFFFFFF
#define	u64MAX      0xFFFFFFFFFFFFFFFF


#define RHEA_NO_COPY_NO_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)


#endif // _rheaDataTypes_h_

