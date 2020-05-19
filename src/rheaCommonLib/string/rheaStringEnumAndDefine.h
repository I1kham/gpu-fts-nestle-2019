#ifndef _rheaStringEnumAndDefine_h_
#define _rheaStringEnumAndDefine_h_
#include "../rheaDataTypes.h"

namespace rhea
{
    //fwd declaration
	class Allocator;


	struct ANSIChar			{ char	data; };
	struct UTF16Char		{ u16	data[2]; };
	struct UTF32Char		{ u32	data; };

	struct UTF8Char			
	{ 
							UTF8Char()											{ setEOF(); }
							UTF8Char(u8 a, u8 b=0, u8 c=0, u8 d=0)				{ setFrom4Byte(a,b,c,d); }
							UTF8Char(const char* const utf8CharSequence)		{ setFromConstChar(utf8CharSequence); }

		void				setEOF()											{ data[0] = data[1] = data[2] = data[3] = 0; }
		bool				isEOF() const										{ return (data[0]==0x00); }
		bool				isEqual (char c) const								{ return (data[0] == c && data[1] == 0x00); }
		bool				isNotEqual (char c) const							{ return !isEqual(c); }
		
		bool				isValidUTF8Sequence() const;
		u8					length() const;
								//ritorna il num di byte necessari a rappresentare il char corrente

		bool				operator==(const UTF8Char& b) const						{ return (memcmp(data, b.data, 4) == 0); }
		bool				operator!=(const UTF8Char& b) const						{ return (memcmp(data, b.data, 4) != 0); }
		bool				operator==(char ansiChar) const							{ return (data[0] == (u8)ansiChar && data[1] == 0x00); }
		bool				operator!=(char ansiChar) const							{ return (data[0] != (u8)ansiChar && data[1] == 0x00); }
		bool				operator==(const char* const utf8CharSequence) const	{ UTF8Char b(utf8CharSequence); return (memcmp(data, b.data, 4) == 0); }
		bool				operator!=(const char* const utf8CharSequence) const	{ UTF8Char b(utf8CharSequence); return (memcmp(data, b.data, 4) != 0); }

		bool				setFrom4Byte (u8 a, u8 b=0, u8 c=0, u8 d=0);
								//true se la sequenza a b c d è valida
		bool				setFromConstChar (const char* const utf8CharSequence, u8 *out_numByteConsumed = NULL);
								/*	true se è riuscito ad estrarre un valido utf8 char.
									Se ritorna true, mette in [out_numByteConsumed] il num di byte di [utf8CharSequence] utilizzati
								*/

		UTF8Char&			operator= (const char* const utf8CharSequence)		{ setFromConstChar(utf8CharSequence); return *this; }
								/* esempio:
									utf8char1 = u8"夜";	

									Se gli passo una stringa, prende solo il primo char
									utf8char1 = u8"ュmiao夜";
									prende solo "ュ", scarta il resto
								*/
		
	public:
		u8					data[4];
	};

} //namespace rhea
#endif //_rheaStringEnumAndDefine_h_

