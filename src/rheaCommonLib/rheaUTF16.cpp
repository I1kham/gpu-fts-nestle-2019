#include "rheaUTF8.h"
#include "rheaUTF16.h"
#include "rheaUtils.h"

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


//***************************************************************
u32 utf16::length(const u16 *s)
{
	if (NULL == s)
		return 0;
	u32 n = 0;
	while (s[n] != 0x00)
		n++;
	return n;
}

//***************************************************************
u32 utf16::rtrim(u16 *s)
{
	u32 n = utf16::length(s);
	if (n == 0)
		return 0;

	while (s[n] == 0x00)
		n--;

	while (s[n] == ' ')
		s[n--] = 0x00;

	return n+1;
}

//***************************************************************
void utf16::concatFromASCII(u16 *dst, u32 sizeofDstInBytes, const char* const src)
{
	if (NULL == src)
		return;
	if (src[0] == 0x00)
		return;

	if (NULL == dst || sizeofDstInBytes == 0)
		return;

	u32 n = utf16::length(dst);
	const u32 nMax = (sizeofDstInBytes / 2) -1;
	u32 i = 0;
	while (src[i] != 0x00 && n<nMax)
		dst[n++] = src[i++];
	dst[n] = 0x00;
}

//***************************************************************
bool utf16::toUTF8(const u16* const src, u8 *out_utf8, u32 *in_out_sizeofOutUTF8)
{
	const u32 len = utf16::length(src);
	if (len == 0)
	{
		if (NULL == out_utf8)
		{
			*in_out_sizeofOutUTF8 = 1;
			return true;
		}

		if (*in_out_sizeofOutUTF8 > 0)
		{
			out_utf8[0] = 0x00;
			*in_out_sizeofOutUTF8 = 1;
			return true;
		}

		*in_out_sizeofOutUTF8 = 1;
		return false;
	}



	bool ret = true;
	u8 tempUTF8Seq[8];
	u32 i2 = 0;
	u32 i = 0;
	while (i < len)
	{
		u32 utf32;
		u8 n = utf16::oneUTF16toUTF32(&src[i], len-i, &utf32);
		if (n == 0)
		{
			//errore durante la decodifica...
			//ritorno quel che ho
			break;
		}

		i += n;

		
		n = utf8::oneUTF32ToUTF8 (utf32, tempUTF8Seq, sizeof(tempUTF8Seq));
		if (n == 0)
		{
			//errore durante la decodifica...
			//ritorno quel che ho
			break;
		}

		if (ret && NULL != out_utf8)
		{
			if (*in_out_sizeofOutUTF8 > i2 + n)
				memcpy(&out_utf8[i2], tempUTF8Seq, n);
			else
				ret = false;
		}

		i2 += n;
	}


	//devo aggiungere 0x00 alla sequenza utf8
	if (ret && NULL != out_utf8)
	{
		if (*in_out_sizeofOutUTF8 > i2)
			out_utf8[i2] = 0x00;
		else
			ret = false;
	}
	++i2;


	*in_out_sizeofOutUTF8 = i2;
	return ret;
}

//***************************************************************
u32 utf16::utf16SequenceToU8Buffer_LSB_MSB (const u16 *utf16_seq, u8 *out_buffer, u32 sizeofOutBuffer, bool bInclude0x00)
{
	u32 i = 0;
	u32 i2 = 0;
	while (utf16_seq[i] && sizeofOutBuffer>=2)
	{
		rhea::utils::bufferWriteU16_LSB_MSB (&out_buffer[i2], utf16_seq[i]);
		i2 += 2;
		sizeofOutBuffer -= 2;
		i++;
	}

	if (bInclude0x00)
	{
		if (sizeofOutBuffer)
			out_buffer[i2++] = 0x00;
		else
		{
			i2 -= 2;
			out_buffer[i2++] = 0x00;
		}
	}

	return i2;
}

//***************************************************************
void utf16::prepend (u16 *dst, u32 sizeOfDstInBytes, const u16* const strToPrepend)
{
	const u32 lenPrepend = utf16::length(strToPrepend);
	u32 lenDst = utf16::length(dst);

	if (0 == lenPrepend)
		return;

	//devo shiftare a destra [dst] di [lenPrepend*2] bytes
	const u32 iStart = lenPrepend * 2;
	u32 nBytesToShift = (lenDst+1) * 2;
	if (sizeOfDstInBytes <= iStart + nBytesToShift)
	{
		nBytesToShift = 0;
		if (sizeOfDstInBytes >= iStart+2)
			nBytesToShift = (sizeOfDstInBytes - iStart) - 2;
	}

	if (nBytesToShift)
		memcpy(&dst[iStart/2], dst, nBytesToShift);

	memcpy(dst, strToPrepend, lenPrepend * 2);

}
