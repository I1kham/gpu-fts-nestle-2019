#ifndef _rheaUTF8String_h_
#define _rheaUTF8String_h_
#include "../rhea.h"
#include "../rheaString.h"
#include "../rheaArray.h"

namespace rhea
{
	namespace utf8
	{
		/*==============================================
		 * String
		 *
		 */
		class String
		{
		public:
								String()																	{ priv_constructor(); }
								String (const String &b);
								String(const u8* const s);
								String(const char* const s);
								~String();

			void				setAllocator (Allocator *allocIN);
			void				prealloc (u32 newSizeInByte);

								//================================================ assign
			void				clear()																		{ curSize = 0; if (buffer) buffer[0] = 0; }
			void				setFrom (const String &b, u32 lenInByte=u32MAX)								{ clear(); append(b, lenInByte); }
			void				setFrom (const u8 *b, u32 lenInByte=u32MAX)									{ clear(); append(b, lenInByte); }
			String&				operator= (const String &b)													{ clear(); append(b); return *this; }
			String&				operator= (const u8 *b)														{ clear(); append(b); return *this; }
			String&				operator= (const char *b)													{ clear(); append(b); return *this; }

								//================================================ append
            void				append (const String &b, u32 lenInByte = u32MAX);
			void				append (const u8 *b, u32 lenInByte = u32MAX);
			void				append (const char *b, u32 lenInByte = u32MAX)								{ append((const u8*)b, lenInByte); }
			void				append (const UTF8Char &b)													{ append (b.data, b.length()); }
			void				append (const char c)														{ const char cc[2] = { c, 0 }; append(cc, 1); }
			void				append (unsigned char c)													{ char buf[4];  sprintf_s (buf, sizeof(buf), "%u", c); append (buf, (u32)strlen(buf)); }
			void				append (int c)																{ char buf[32]; sprintf_s (buf, sizeof(buf), "%d", c); append (buf, (u32)strlen(buf)); }
			void				append (unsigned int c)														{ char buf[32]; sprintf_s (buf, sizeof(buf), "%u", c); append (buf, (u32)strlen(buf)); }
#ifdef WIN32
            void				append (long c)																{ char buf[32]; sprintf_s (buf, sizeof(buf), "%d", c); append (buf, (u32)strlen(buf)); }
            void				append (unsigned long c)													{ char buf[32]; sprintf_s (buf, sizeof(buf), "%u", c); append (buf, (u32)strlen(buf)); }
			void				append(u64 i)																{ char buf[64]; sprintf_s(buf, sizeof(buf), "%I64u", i); append(buf, (u32)strlen(buf)); }
			void				append(i64 i)																{ char buf[64]; sprintf_s(buf, sizeof(buf), "%I64i", i); append(buf, (u32)strlen(buf)); }
#else
            void				append (u64 i)																{ char buf[64]; sprintf_s (buf, sizeof(buf),"%" PRIu64,i); append (buf, (u32)strlen(buf)); }
            void				append (i64 i)																{ char buf[64]; sprintf_s (buf, sizeof(buf),"%" PRIi64,i); append (buf, (u32)strlen(buf)); }
#endif

			friend	String&		operator<<  (String &me, const String &b)									{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, const u8 *b)										{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, const char *b)										{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, const UTF8Char &b)									{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, char b)											{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, unsigned char b)									{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, int b)												{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, unsigned int b)									{ me.append (b); return me; }
#ifdef WIN32
			friend	String&		operator<<  (String &me, long b)											{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, unsigned long b)									{ me.append (b); return me; }
#endif
			friend	String&		operator<<  (String &me, u64 b)												{ me.append (b); return me; }
			friend	String&		operator<<  (String &me, i64 b)												{ me.append (b); return me; }

								//================================================ concat
			static	String		concat (const String &a, const String &b)									{ String ret; ret.prealloc (a.lengthInBytes() + b.lengthInBytes()+1); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, const String &b)								{ return String::concat (a, b); }
			friend	String		operator&  (const String &a, const UTF8Char &b)								{ String ret; ret.prealloc (a.lengthInBytes() + 2); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, const char *b)									{ String ret; u32 n=(u32)strlen(b); ret.prealloc (a.lengthInBytes() + n +1); ret = a; ret.append(b, n); return ret; }
			friend	String		operator&  (const String &a, char b)										{ String ret; ret.prealloc (a.lengthInBytes() + 2); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, unsigned char b)								{ String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, int b)											{ String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, unsigned int b)								{ String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
#ifdef WIN32
            friend	String		operator&  (const String &a, long b)										{ String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
			friend	String		operator&  (const String &a, unsigned long b)								{ String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
#endif
            friend	String		operator&  (const String &a, u64 b)                                         { String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }
            friend	String		operator&  (const String &a, i64 b)                                         { String ret; ret.prealloc (a.lengthInBytes() + 16); ret = a; ret.append(b); return ret; }


								//================================================ query
			u32					lengthInBytes() const														{ return curSize; }
			u8*					getBuffer() const															{ return buffer; }
			i32					findFirst (const rhea::UTF8Char &ch, u32 startIndex = 0) const;
			bool				isEqualTo (const String &b, bool bCaseSensitive) const;
			bool				isEqualTo (const u8 * const b, bool bCaseSensitive) const;
			bool				isEqualToWithLen (const String &b, u32 lenInBytes, bool bCaseSensitive) const;
			bool				isEqualToWithLen (const u8* const b, u32 lenInBytes, bool bCaseSensitive) const;

								//================================================ utils
			u32					explode (const UTF8Char &cTofind, Array<String> &out) const;
			void				trim()																		{ trimR(); trimL(); }
			void				trimL();
			void				trimR();
			void				sanitizePath();

		private:
			void				priv_constructor();

		private:
			Allocator		*allocator;
			u8				*buffer;
			u32				allocatedSize;
			u32				curSize;
			
		};

	} //namespace utf8
}//namespace rhea
#endif //_rheaUTF8String_h_
