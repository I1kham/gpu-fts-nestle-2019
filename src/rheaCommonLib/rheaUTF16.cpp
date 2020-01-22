#include "rheaUTF16.h"

using namespace rhea;

#define UTF16_UNI_SUR_HIGH_START	0xD800
#define UTF16_UNI_SUR_HIGH_END		0xDBFF
#define UTF16_UNI_SUR_LOW_START		0xDC00
#define UTF16_UNI_SUR_LOW_END		0xDFFF
#define UTF16_UNI_MAX_LEGAL_UTF32	0x0010FFFF
#define UTF16_HALFBASE				0x00010000

//***************************************************************
u8 utf16::oneUTF16toUTF32(const u16 *utf16Sequence, u32 n_u16_in_src, u32 *out_utf32)
{
	if (n_u16_in_src == 0 || NULL == utf16Sequence)
		return 0;

	if ((utf16Sequence[0] & 0xFC00) != 0xD800)
	{
		*out_utf32 = (u32)utf16Sequence[0];
		return 1;
	}

	//se arriviamo qui, vuol dire che la codifica prevede l'utilizzo di 2 u16
	if (n_u16_in_src < 2)
		return 0;
	if ((utf16Sequence[1] & 0xFC00) != 0xDC00)
		return 0;

	*out_utf32 = (utf16Sequence[0] & 0x03FF);
	*out_utf32 <<= 10;
	*out_utf32 |= (utf16Sequence[1] & 0x03FF);
	return 2;
}

//***************************************************************
u8 utf16::oneUTF32toUTF16 (u32 utf32, u16 *out, u8 num_of_u16_in_out)
{
	if (utf32 <= 0xFFFF)
	{ 
		/* UTF-16 surrogate values are illegal in UTF-32; 0xffff or 0xfffe are both reserved values */
		if (utf32 >= UTF16_UNI_SUR_HIGH_START && utf32 <= UTF16_UNI_SUR_LOW_END)
			return 0;
		else 
		{
			*out = (u16)utf32;
			return 1;
		}
	}

	if (utf32 > UTF16_UNI_MAX_LEGAL_UTF32)
		return 0;

	/* target is a character in range 0xFFFF - 0x10FFFF. */
	if (num_of_u16_in_out < 2)
		return 0;

	utf32 -= UTF16_HALFBASE;
	out[0] = ((utf32 >> 10) + UTF16_UNI_SUR_HIGH_START);
	out[1] = ((utf32 & 0x000003FF) + UTF16_UNI_SUR_LOW_START);
	return 2;
}

