#ifndef _rheaDataTypes_h_
#define _rheaDataTypes_h_
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>	//for memcpy

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


namespace rhea
{
	enum eEndianess
	{
		eLittleEndian = 0,
		eBigEndian = 1
	};

	enum eSeek
	{
		eSeekStart = 0,
		eSeekEnd = 1,
		eSeekCurrent = 2
	};

	/*===============================================================
	 * endian conversion
	 *==============================================================*/
	inline u16		endianConvertU16(u16 w)								{ return ((w >> 8) & 0x00FF) | ((w << 8) & 0xFF00); }
	inline i16		endianConvertI16(i16 w)								{ return ((w >> 8) & 0x00FF) | ((w << 8) & 0xFF00); }
	inline u32		endianConvertU32(u32 w)								{ return ((w >> 24) & 0x000000FF) | ((w >> 8) & 0x0000FF00) | ((w << 8) & 0x00FF0000) | ((w << 24) & 0xFF000000); }
	inline i32		endianConvertI32(i32 w)								{ return ((w >> 24) & 0x000000FF) | ((w >> 8) & 0x0000FF00) | ((w << 8) & 0x00FF0000) | ((w << 24) & 0xFF000000); }
	inline u64		endianConvertU64(u64 w)								{ return ((w & 0x00000000000000ff) << 56) | ((w & 0x000000000000ff00) << 40) | ((w & 0x0000000000ff0000) << 24) | ((w & 0x00000000ff000000) << 8) | ((w & 0x000000ff00000000) >> 8) | ((w & 0x0000ff0000000000) >> 24) | ((w & 0x00ff000000000000) >> 40) | ((w & 0xff00000000000000) >> 56); }
	inline i64		endianConvertI64(u64 w)								{ return ((w & 0x00000000000000ff) << 56) | ((w & 0x000000000000ff00) << 40) | ((w & 0x0000000000ff0000) << 24) | ((w & 0x00000000ff000000) << 8) | ((w & 0x000000ff00000000) >> 8) | ((w & 0x0000ff0000000000) >> 24) | ((w & 0x00ff000000000000) >> 40) | ((w & 0xff00000000000000) >> 56); }

#ifdef LINUX
    #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif
	inline f32		endianConvertF32(f32 f)								{ u32 w = ((u32*)&f)[0]; u32 t = ((w >> 24) & 0x000000FF) | ((w >> 8) & 0x0000FF00) | ((w << 8) & 0x00FF0000) | ((w << 24) & 0xFF000000);  f32 ret; memcpy(&ret, &t, 4); return ret; }
}; //namespace rhea

#endif // _rheaDataTypes_h_

