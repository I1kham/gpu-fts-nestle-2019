#ifndef _rheaNetBufferView_h_
#define _rheaNetBufferView_h_
#include "rheaDataTypes.h"
#include "rheaStaticBuffer.h"

namespace rhea
{
	/******************************************************************************
	 * NetBufferViewR
	 *
	 * Utilizza un generico template Buffer e implementa una interfaccia di lettura "file like"
	 * setup() definisce la zona di memoria da gestire, a partire da offset, per un totale di numBytes
	 * I read falliscono se si prova a leggere oltre alla posizione numBytes
	 *	read legge dalla pos corrente e la incrementa
	 *	readAt invece legge dalla pos indicata e lascia inalterata la pos corrente
	 *
	 * La peculiarità di questa classe, è che salva i dati in formato Bigendian/Littleendian a seconda 
	 * del parametro indicato in setup
	 * Se [endianessOfTheBuffer] è Littleendian e la platform è a sua volta Littleendian, allora non viene effttuata nessuna conversione.
	 * Se [endianessOfTheBuffer] è Littleendian e la platform è Bigendian, allora viene effettuata la conversione.
	 * Vale anche il contrario (platform=Bigendian, endianess==Littleendian)
	 *******************************************************************************/
	template <class TBuffer>
	class NetBufferViewR
	{
	public:
							NetBufferViewR()													{ buffer = NULL; bufferOffset = bufferNumBytes = cursor = 0; }

		void				setup (const TBuffer *bufferIN, u32 offsetIN, u32 numBytesIN, eEndianess endianessOfTheBuffer)
							{
								assert (bufferIN && numBytesIN);
								assert (offsetIN + numBytesIN <= bufferIN->getTotalSizeAllocated());
								buffer = bufferIN;
								bufferOffset = offsetIN;
								bufferNumBytes = numBytesIN;
								cursor = 0;

								if (endianessOfTheBuffer == rhea::eEndianess::eLittleEndian)
								{
									if (rhea::isLittleEndian())
										bNeedendianConversion = false;
									else
										bNeedendianConversion = true;
								}
								else
								{
									if (rhea::isLittleEndian())
										bNeedendianConversion = true;
									else
										bNeedendianConversion = false;
								}
							}

		void				reset ()															{ cursor = 0; }
		u32					tell() const														{ return cursor; }
		void				seek (u32 pos, eSeek from = eSeek::start)
							{
								switch (from)
								{
								case rhea::eSeek::start:
									cursor = pos;
									if (cursor >= bufferNumBytes)
										cursor = bufferNumBytes;
									break;

								case rhea::eSeek::current:
									cursor += pos;
									if (cursor >= bufferNumBytes)
										cursor = bufferNumBytes;
									break;

								case rhea::eSeek::end:
									if (pos >= bufferNumBytes)
										cursor = 0;
									else
										cursor = bufferNumBytes - pos;
									break;
								}
							}

		u32					length() const														{ return bufferNumBytes; }
			
		bool				readU8 (u8 &out)													{ return readBlob (&out, sizeof(out)); }
		bool				readI8 (i8 &out)													{ return readBlob (&out, sizeof(out)); }
		bool				readU16 (u16 &out)													{ u16 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU16(temp); return true;}
		bool				readI16 (i16 &out)													{ i16 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI16(temp); return true;}
		bool				readU32 (u32 &out)													{ u32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU32(temp); return true;}
		bool				readI32 (i32 &out)													{ i32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI32(temp); return true;}
		bool				readU64 (u64 &out)													{ u64 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU64(temp); return true;}
		bool				readI64 (i64 &out)													{ i64 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI64(temp); return true;}
		bool				readF32 (f32 &out)													{ f32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertF32(temp); return true;}
		bool				readBlob (void *dst, u32 numBytesToread)							{ if (!readBlobAt (dst, numBytesToread, cursor)) return false;  cursor+=numBytesToread; return true; }
			
		bool				readU8At (u8 &out, u32 offset) const								{ return readBlobAt (&out, sizeof(out), offset); }
		bool				readI8At (i8 &out, u32 offset) const								{ return readBlobAt (&out, sizeof(out), offset); }
		bool				readU16At (u16 &out, u32 offset) const								{ u16 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU16(temp); return true; }
		bool				readI16At (i16 &out, u32 offset) const								{ i16 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI16(temp); return true; }
		bool				readU32At (u32 &out, u32 offset) const								{ u32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU32(temp); return true; }
		bool				readI32At (i32 &out, u32 offset) const								{ i32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI32(temp); return true; }
		bool				readU64At (u64 &out, u32 offset) const								{ u64 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU64(temp); return true; }
		bool				readI64At (i64 &out, u32 offset) const								{ i64 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI64(temp); return true; }
		bool				readF32At (f32 &out, u32 offset) const								{ f32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertF32(temp); return true; }
		bool				readBlobAt (void *dst, u32 numBytesToread, u32 offset)	const		{ assert (offset + numBytesToread  <= bufferNumBytes); return buffer->read (dst, bufferOffset + offset, numBytesToread); }

	private:
							RHEA_NO_COPY_NO_ASSIGN(NetBufferViewR);
		u16					convertU16 (u16 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU16(w); }
		i16					convertI16 (i16 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI16(w); }
		u32					convertU32 (u32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU32(w); }
		i32					convertI32 (i32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI32(w); }
		u64					convertU64 (u64 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU64(w); }
		i64					convertI64 (i64 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI64(w); }
		f32					convertF32 (f32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertF32(w); }

	private:
		const TBuffer		*buffer;
		u32					bufferOffset, bufferNumBytes;
		u32					cursor;
		bool				bNeedendianConversion;
	};


	/******************************************************************************
	 * NetBufferViewW
	 *
	 * Utilizza un generico template Buffer e implementa una interfaccia di lettura/scrittura "file like"
	 * setup() definisce la zona di memoria da gestire, a partire da offset. Non c'è limite superiore imposto
	 * ma le write falliscono se si supera l'attuale dimensione del buffer e bCangrow=false.
	 * Se invece bCangrow=true, allora il buffer viene espanso
	 *	write/read scrive/legge dalla pos corrente e la incrementa
	 *	writeAt/readAt invece scrive/legge dalla pos indicata e lascia inalterata la pos corrente
	 *
	 * Mano mano che si fanno delle write, il contatore interno bufferBytesWritten viene incrementato per riflettere la posizione del write
	 *	più "in alto". In pratica se fai un writeAt (offset=10, len=5), bufferBytesWritten diventa 15.
	 *
	 * La peculiarità di questa classe, è che salva i dati in formato Bigendian/Littleendian a seconda 
	 * del parametro indicato in setup
	 * Se [endianessOfTheBuffer] è Littleendian e la platform è a sua volta Littleendian, allora non viene effttuata nessuna conversione.
	 * Se [endianessOfTheBuffer] è Littleendian e la platform è Bigendian, allora viene effettuata la conversione.
	 * Vale anche il contrario (platform=Bigendian, endianess==Littleendian)
	 *******************************************************************************/
	template <class TBuffer>
	class NetBufferViewW
	{
	public:
							NetBufferViewW()													{ buffer = NULL; bufferOffset = bufferBytesWritten = cursor = 0; }

		void				setup (TBuffer *bufferIN, u32 offsetIN, eEndianess endianessOfTheBuffer, bool bufferCangrowIN)
							{
								assert (bufferIN);
								buffer = bufferIN;
								bufferOffset = offsetIN;
								bufferCangrow = bufferCangrowIN;
								bufferBytesWritten = 0;
								cursor = 0;

								if (endianessOfTheBuffer == rhea::eEndianess::eLittleEndian)
								{
									if (rhea::isLittleEndian())
										bNeedendianConversion = false;
									else
										bNeedendianConversion = true;
								}
								else
								{
									if (rhea::isLittleEndian())
										bNeedendianConversion = true;
									else
										bNeedendianConversion = false;
								}
							}

		void				reset ()															{ cursor = bufferBytesWritten = 0; }
		u32					tell() const														{ return cursor; }
		void				seek (u32 pos, eSeek from = eSeek::start)
							{
								switch (from)
								{
								case rhea::eSeek::start:
									cursor = pos;
									break;

								case rhea::eSeek::current:
									cursor += pos;
									break;

								case rhea::eSeek::end:
									if (pos >= bufferBytesWritten)
										cursor = 0;
									else
										cursor = bufferBytesWritten - pos;
									break;
								}
							}

		u32					length() const														{ return bufferBytesWritten; }

		bool				readU8 (u8 &out)													{ return readBlob (&out, sizeof(out)); }
		bool				readI8 (i8 &out)													{ return readBlob (&out, sizeof(out)); }
		bool				readU16 (u16 &out)													{ u16 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU16(temp); return true;}
		bool				readI16 (i16 &out)													{ i16 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI16(temp); return true;}
		bool				readU24 (u32 &out)													{ u32 temp; if (!readBlob(&temp, 3)) return false; out = convertU32(temp); return true; }
		bool				readI24 (i32 &out)													{ i32 temp; if (!readBlob(&temp, 3)) return false; out = convertI32(temp); return true; }
		bool				readU32 (u32 &out)													{ u32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU32(temp); return true;}
		bool				readI32 (i32 &out)													{ i32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI32(temp); return true;}
		bool				readU64 (u64 &out)													{ u64 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertU64(temp); return true;}
		bool				readI64 (i64 &out)													{ i64 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertI64(temp); return true;}
		bool				readF32 (f32 &out)													{ f32 temp; if (!readBlob(&temp, sizeof(temp))) return false; out=convertF32(temp); return true;}
		bool				readBlob (void *dst, u32 numBytesToread)							{ if (!readBlobAt (dst, numBytesToread, cursor)) return false;  cursor+=numBytesToread; return true; }
			
		bool				readU8At (u8 &out, u32 offset) const								{ return readBlobAt (&out, sizeof(out), offset); }
		bool				readI8At (i8 &out, u32 offset) const								{ return readBlobAt (&out, sizeof(out), offset); }
		bool				readU16At (u16 &out, u32 offset) const								{ u16 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU16(temp); return true; }
		bool				readI16At (i16 &out, u32 offset) const								{ i16 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI16(temp); return true; }
		bool				readU32At (u32 &out, u32 offset) const								{ u32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU32(temp); return true; }
		bool				readI32At (i32 &out, u32 offset) const								{ i32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI32(temp); return true; }
		bool				readU64At (u64 &out, u32 offset) const								{ u64 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertU64(temp); return true; }
		bool				readI64At (i64 &out, u32 offset) const								{ i64 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertI64(temp); return true; }
		bool				readF32At (f32 &out, u32 offset) const								{ f32 temp; if (!readBlobAt (&temp, sizeof(temp), offset)) return false; out = convertF32(temp); return true; }
		bool				readBlobAt (void *dst, u32 numBytesToread, u32 offset) const		{ assert (offset + numBytesToread  <= bufferBytesWritten); return buffer->read (dst, bufferOffset + offset, numBytesToread); }

		bool				writeU8 (u8 src)													{ return writeBlob (&src, sizeof(src)); }
		bool				writeI8 (i8 src)													{ return writeBlob (&src, sizeof(src)); }
		bool				writeU16 (u16 src)													{ u16 temp=convertU16(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeI16 (i16 src)													{ i16 temp=convertI16(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeU24 (u32 src)													{ u32 temp = convertU32(src); return writeBlob(&temp, 3); }
		bool				writeI24 (i32 src)													{ i32 temp = convertI32(src); return writeBlob(&temp, 3); }
		bool				writeU32 (u32 src)													{ u32 temp=convertU32(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeI32 (i32 src)													{ i32 temp=convertI32(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeU64 (u64 src)													{ u64 temp=convertU64(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeI64 (i64 src)													{ u64 temp=convertI64(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeF32 (f32 src)													{ f32 temp=convertF32(src); return writeBlob(&temp, sizeof(temp)); }
		bool				writeBlob (const void *src, u32 numBytesTowrite)					{ if (!writeBlobAt (src, numBytesTowrite, cursor)) return false;  cursor+=numBytesTowrite; return true; }

		bool				writeU8At (u8 src, u32 offset)										{ return writeBlobAt (&src, sizeof(src), offset); }
		bool				writeI8At (i8 src, u32 offset)										{ return writeBlobAt (&src, sizeof(src), offset); }
		bool				writeU16At (u16 src, u32 offset)									{ u16 temp=convertU16(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeI16At (i16 src, u32 offset)									{ i16 temp=convertI16(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeU32At (u32 src, u32 offset)									{ u32 temp=convertU32(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeI32At (i32 src, u32 offset)									{ i32 temp=convertI32(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeU64At (u64 src, u32 offset)									{ u64 temp=convertU64(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeI64At (i64 src, u32 offset)									{ u64 temp=convertI64(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeF32At (f32 src, u32 offset)									{ f32 temp=convertF32(src); return writeBlobAt(&temp, sizeof(temp), offset); }
		bool				writeBlobAt (const void *src, u32 numBytesTowrite, u32 offset)		{ if (!buffer->write (src, bufferOffset + offset, numBytesTowrite, bufferCangrow)) return false; u32 d = offset + numBytesTowrite; if (d > bufferBytesWritten) bufferBytesWritten = d; return true; }

	private:
							RHEA_NO_COPY_NO_ASSIGN(NetBufferViewW);
		u16					convertU16 (u16 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU16(w); }
		i16					convertI16 (i16 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI16(w); }
		u32					convertU32 (u32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU32(w); }
		i32					convertI32 (i32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI32(w); }
		u64					convertU64 (u64 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertU64(w); }
		i64					convertI64 (i64 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertI64(w); }
		f32					convertF32 (f32 w) const											{ if (!bNeedendianConversion) return w; return rhea::endianConvertF32(w); }

	private:
		TBuffer				*buffer;
		u32					bufferOffset, bufferBytesWritten;
		u32					cursor;
		bool				bufferCangrow, bNeedendianConversion;
	};










	/*********************************************************
	*  Classe di comodo, accetta direttamente un void* buffer come parametro invece che un TBuffer
	*
	*/
	class NetStaticBufferViewR : public NetBufferViewR<rhea::StaticBufferReadOnly>
	{
	public:
					NetStaticBufferViewR() : NetBufferViewR() {}

		void		setup(const void *bufferIN, u32 sizeOfBuffer, eEndianess endianessOfTheBuffer)
					{
						sb.setup(bufferIN, sizeOfBuffer);
						NetBufferViewR<rhea::StaticBufferReadOnly>::setup(&sb, 0, sizeOfBuffer, endianessOfTheBuffer);
					}

	private:
		StaticBufferReadOnly	sb;
	};


	/*********************************************************
	*  Classe di comodo, accetta direttamente un void* buffer come parametro invece che un TBuffer
	*/
	class NetStaticBufferViewW : public NetBufferViewW<rhea::StaticBuffer>
	{
	public:
					NetStaticBufferViewW() : NetBufferViewW() {}

		void		setup(void *bufferIN, u32 sizeOfBuffer, eEndianess endianessOfTheBuffer)
					{
						sb.setup(bufferIN, sizeOfBuffer);
						NetBufferViewW<rhea::StaticBuffer>::setup(&sb, 0, endianessOfTheBuffer, false);
					}

	private:
		StaticBuffer	sb;
	};





}; //namespace rhea
#endif // _rheaNetBufferView_h_