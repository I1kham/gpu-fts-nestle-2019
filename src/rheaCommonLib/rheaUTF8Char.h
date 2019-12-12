#ifndef _rheaUTF8Char_h_
#define _rheaUTF8Char_h_
#include "rheaDataTypes.h"
#include <stdio.h>
#include <string.h>


namespace rhea
{
	namespace utf8
	{
		/*===============================================
		 *
		 *==============================================*/
		class Char
		{
		public:
						Char ()												{ memset(c,0,4); }
						Char (const Char &b)								{ setFrom(b); }
						Char (char b)										{ setFrom(b); }
						Char (const char *b)								{ assertStringIsComposedByOnlyOneUTF8Char(b); setFrom(b, 4); }

						//========================= assign
			Char&		operator= (const char b)							{ assert ((b & 0x80)==0); setFrom(b); return *this; }
			Char&		operator= (const char *b)							{ assertStringIsComposedByOnlyOneUTF8Char(b); setFrom(b); return *this; }
			Char&		operator= (const Char &b)							{ setFrom(b); return *this; }
			void		setNULL ()											{ memset(c,0,4); }
			void		setFrom (const Char &b)								{ memcpy(c, b.c, 4); }
			void		setFrom (char b)									{ assert ((b & 0x80)==0); memset(c,0,4); c[0]=b; }
			u8			setFrom (const char *src, u32 srclenInByte);
						/*	estrae il primo utf8char da *src.
							Ritorna il num di byte della sequenza, oppure 0 se la seq è invalida
						*/
			u8			setFromRev (const char *src, u32 srclenInByte);
						/*	parte da src[srclenInByte-1] e cerca all'indietro il primo utf8char.
							Ritorna il num di byte della sequenza, oppure 0 se la seq è invalida
						*/
			void		setFromUTF32 (u32 u);

						
						//========================= comparison
			bool		operator== (const Char &b) const					{ if (memcmp (c, b.c, 4) == 0) return true; return false; }
			bool		operator== (char b) const							{ if (isAscii() && c[0]==b) return true; return false; }
			bool		operator>  (const Char &b) const					{ if (memcmp (c, b.c, 4) > 0 ) return true; return false; }
			bool		operator> (char b) const							{ if (isAscii() && c[0]>b) return true; return false; }
			bool		operator>= (const Char &b) const					{ if (memcmp (c, b.c, 4) >=0 ) return true; return false; }
			bool		operator>= (char b) const							{ if (isAscii() && c[0]>=b) return true; return false; }
			bool		operator<  (const Char &b) const					{ if (memcmp (c, b.c, 4) < 0 ) return true; return false; }
			bool		operator< (char b) const							{ if (isAscii() && c[0]<b) return true; return false; }
			bool		operator<= (const Char &b) const					{ if (memcmp (c, b.c, 4) <=0 ) return true; return false; }
			bool		operator<= (char b) const							{ if (isAscii() && c[0]<=b) return true; return false; }
			bool		operator!= (const Char &b) const					{ if ((*this) == b) return false; return true; }
			bool		operator!= (char b) const							{ if ((*this) == b) return false; return true; }
			bool		isEqualNoCaseSensitive (const Char &b) const;

						//=========================query
			u8			len () const										{ if(!c[0]) return 0; if(!c[1]) return 1; if(!c[2]) return 2; if(!c[3]) return 3; return 4;}
			const u8*	getBuffer() const									{ return c; }
			u32			toUTF32 () const;
			bool		isNULL() const										{ return (c[0] == 0x00); }
			bool		isAscii() const										{ return ((c[0] & 0x80) == 0); }
			bool		isCharMaiuscolo ()	const							{ if (c[1]==0 && c[0]>='A' && c[0]<='Z') return true;  return false; }
			bool		isCharMinuscolo ()	const							{ if (c[1]==0 && c[0]>='a' && c[0]<='z') return true; return false; }
			bool		isANumber () const									{ if (c[1]==0 && c[0]>='0' && c[0]<='9') return true;  return false; }
			bool		isALetter () const									{ if (c[1]==0 && ((c[0]>='a' && c[0]<='z') || (c[0]>='A' && c[0]<='Z'))) return true; return false; }
			bool		isALetterOrANumber () const							{ if (c[1]==0 && ((c[0]>='a' && c[0]<='z') || (c[0]>='A' && c[0]<='Z') || (c[0]>='0' && c[0]<='9'))) return true; return false; }

						
		private:
#ifdef _DEBUG
			void		assertStringIsComposedByOnlyOneUTF8Char (const char *c) const;
#else
			void		assertStringIsComposedByOnlyOneUTF8Char (const char *c) const		{}
#endif
		private:
			u8			c[4];
		};
	}; //namespace utf8
};//namespace rhea
#endif //_rheaUTF8Char_h_