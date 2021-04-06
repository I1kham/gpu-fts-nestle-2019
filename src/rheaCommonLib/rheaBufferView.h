#ifndef _rheaBufferView_h_
#define _rheaBufferView_h_
#include "rheaDataTypes.h"


namespace rhea
{
	/******************************************************************************
	 * BufferViewR
	 *
	 * Utilizza un generico template Buffer e implementa una interfaccia di lettura "file like"
	 * setup() definisce la zona di memoria da gestire, a partire da offset, per un totale di numBytes
	 * I read falliscono se si prova a leggere oltre alla posizione numBytes
	 *	read legge dalla pos corrente e la incrementa
	 *	readAt invece legge dalla pos indicata e lascia inalterata la pos corrente
	 *******************************************************************************/
	template <class TBuffer>
	class BufferViewR
	{
	public:
							BufferViewR()													{ buffer = NULL; bufferOffset = bufferNumBytes = cursor = 0; }

		void				setup (const TBuffer *bufferIN, u32 offsetIN, u32 numBytesIN)	{ assert (bufferIN && numBytesIN); assert (offsetIN + numBytesIN <= bufferIN->getTotalSizeAllocated()); buffer = bufferIN; bufferOffset = offsetIN; bufferNumBytes = numBytesIN; cursor = 0; }

		void				reset ()														{ cursor = 0; }
		u32					tell() const													{ return cursor; }
		bool				read (void *dst, u32 numBytesToread)							{ if (!readAt (dst, numBytesToread, cursor)) return false;  cursor+=numBytesToread; return true; }
		bool				readAt (void *dst, u32 numBytesToread, u32 offset) const		{ assert (offset + numBytesToread  <= bufferNumBytes); return buffer->read (dst, bufferOffset + offset, numBytesToread); }
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

	private:
							RHEA_NO_COPY_NO_ASSIGN(BufferViewR);

	private:
		const TBuffer		*buffer;
		u32					bufferOffset, bufferNumBytes;
		u32					cursor;
	};



	/******************************************************************************
	 * BufferViewW
	 *
	 * Utilizza un generico template Buffer e implementa una interfaccia di scrittura "file like"
	 * setup() definisce la zona di memoria da gestire, a partire da offset. Non c'è limite superiore imposto
	 * ma le write falliscono se si supera l'attuale dimensione del buffer e bCangrow=false.
	 * Se invece bCangrow=true, allora il buffer viene espanso
	 *	write/read scrive/legge dalla pos corrente e la incrementa
	 *	writeAt/readAt invece scrive/legge dalla pos indicata e lascia inalterata la pos corrente
	 *******************************************************************************/
	template <class TBuffer>
	class BufferViewW
	{
	public:
							BufferViewW()															{ buffer = NULL; bufferOffset = cursor = bufferBytesWritten = 0; bufferCangrow = false; }

		void				setup (TBuffer *bufferIN, u32 offsetIN, bool bCangrowIN = true)			{ assert (bufferIN); buffer = bufferIN; bufferOffset = offsetIN; bufferCangrow = bCangrowIN; cursor = bufferBytesWritten = 0; }

		void				reset ()																{ cursor = bufferBytesWritten = 0; }
		u32					tell() const															{ return cursor; }
		u32					length() const															{ return bufferBytesWritten; }
			
		bool				read (void *dst, u32 numBytesToread)									{ if (!readAt (dst, numBytesToread, cursor)) return false;  cursor+=numBytesToread; return true; }
		bool				readAt (void *dst, u32 numBytesToread, u32 offset) const				{ assert (offset + numBytesToread  <= buffer->getTotalSizeAllocated()); return buffer->read (dst, bufferOffset + offset, numBytesToread); }
		bool				write (const void *src, u32 numBytesTowrite)							{ if (!writeAt (src, numBytesTowrite, cursor)) return false;  cursor+=numBytesTowrite; return true; }
		bool				writeAt (const void *src, u32 numBytesTowrite, u32 offset)				{ if (!buffer->write (src, bufferOffset + offset, numBytesTowrite, bufferCangrow)) return false; u32 d = offset + numBytesTowrite; if (d > bufferBytesWritten) bufferBytesWritten = d; return true; }

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

		 const TBuffer*		_queryBuffer() const													{ return buffer; }

	private:
							RHEA_NO_COPY_NO_ASSIGN(BufferViewW);

	private:
		TBuffer				*buffer;
		u32					bufferOffset, bufferBytesWritten;
		u32					cursor;
		bool				bufferCangrow;
	};
}; //namespace rhea
#endif //_rheaBufferView_h_