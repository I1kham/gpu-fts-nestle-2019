#include "rheaUTF8.h"
#include "rheaUTF8Char.h"

using namespace rhea;


//***************************************************************
bool utf8::extractOneUTF8Char (const char *src, u32 srcLen, u8 *outNumUsedBytes)
{
	if (NULL == src || srcLen<=0)
		return false;

	if ((src[0] & 0x80) == 0)
	{
		*outNumUsedBytes = 1;
		return true;
	}

	//a questo punto il char UTF8 consiste di almeno 2 byte
	if (srcLen < 2)
		return false;
	

	//c1 deve essere di tipo 0x10xxxxxx
	if ((src[1] & 0xC0) != 0x80)
		return false;
	
	if ((src[0]  & 0xE0) == 0xC0)
	{
		//siamo nel caso c0=110xxxxx c1=10xxxxxx
		*outNumUsedBytes = 2;
		return true;
	}


	else if ((src[0]  & 0xF0) == 0xE0)
	{
		//siamo nel caso src[0]=1110xxxx src[1]=10xxxxxx src[2]=10xxxxxx
		if (srcLen < 3)
			return false;
		
		//src[2] deve essere di tipo 0x10xxxxxx
		if ((src[2] & 0xC0) != 0x80)
			return false;	

		*outNumUsedBytes = 3;
		return true;
	}


	else if ((src[0]  & 0xF8) == 0xF0)
	{
		//siamo nel caso src[0]=11110xxx src[1]=10xxxxxx src[2]=10xxxxxx src[3]=10xxxxxx
		if (srcLen < 4)
			return false;
		
		//src[2] deve essere di tipo 0x10xxxxxx
		if ((src[2] & 0xC0) != 0x80)
			return false;
		
		//src[3] deve essere di tipo 0x10xxxxxx
		if ((src[3] & 0xC0) != 0x80)
			return false;

		*outNumUsedBytes = 4;
		return true;
	}

	return false;
}


//***************************************************************
u8 utf8::detectCharlenInByteByFirstByte (char c)
{
	if ((c & 0x80) == 0)
		return 1;
	
	if ((c  & 0xE0) == 0xC0)
		//siamo nel caso c0=110xxxxx c1=10xxxxxx
		return 2;


	if ((c  & 0xF0) == 0xE0)
		//siamo nel caso src[0]=1110xxxx src[1]=10xxxxxx src[2]=10xxxxxx
		return 3;


	else if ((c  & 0xF8) == 0xF0)
		//siamo nel caso src[0]=11110xxx src[1]=10xxxxxx src[2]=10xxxxxx src[3]=10xxxxxx
		return 4;
	
	return 0;
}

//***************************************************************
bool utf8::extractOneUTF8CharRev (const char *src, u32 srcLen, u8 *outNumUsedBytes)
{
	assert (NULL != src && srcLen>0);

	--srcLen;
	if (utf8::isAValidUTF8Char (&src[srcLen], 1))
	{
		*outNumUsedBytes = 1;
		return true;
	}

	if (srcLen-- == 0)
	 return false;
	if (utf8::isAValidUTF8Char (&src[srcLen], 2))
	{
		*outNumUsedBytes = 2;
		return true;
	}

	if (srcLen-- == 0)
		return false;
	if (utf8::isAValidUTF8Char (&src[srcLen], 3))
	{
		*outNumUsedBytes = 3;
		return true;
	}

	if (srcLen-- == 0)
		return false;
	if (utf8::isAValidUTF8Char (&src[srcLen], 4))
	{
		*outNumUsedBytes = 4;
		return true;
	}

	return false;
}

//***************************************************************
bool utf8::isAValidUTF8Char (const char *src, u32 nBytes)
{
	assert (NULL != src && nBytes>0 && nBytes<5);

	char c0 = src[0];
	if ((c0 & 0x80) == 0)
	{
		if (nBytes == 1)
			return true;
		return false;
	}

	//a questo punto il char UTF8 consiste di almeno 2 byte
	if (nBytes < 2)
		return false;
	

	//c1 deve essere di tipo 0x10xxxxxx
	char c1 = src[1];
	if ((c1 & 0xC0) != 0x80)
		return false;
	
	if ((c0  & 0xE0) == 0xC0)
	{
		//siamo nel caso c0=110xxxxx c1=10xxxxxx
		if (nBytes == 2)
			return true;
		return false;
	}


	else if ((c0  & 0xF0) == 0xE0)
	{
		//siamo nel caso c0=1110xxxx c1=10xxxxxx c2=10xxxxxx
		if (nBytes < 3)
			return false;
		
		//c2 deve essere di tipo 0x10xxxxxx
		char c2 = src[2];
		if ((c2 & 0xC0) != 0x80)
			return false;	

		if (nBytes==3)
			return true;
		return false;
	}


	else if ((c0  & 0xF8) == 0xF0)
	{
		//siamo nel caso c0=11110xxx c1=10xxxxxx c2=10xxxxxx c3=10xxxxxx
		if (nBytes < 4)
			return false;
		
		//c2 deve essere di tipo 0x10xxxxxx
		char c2 = src[2];
		if ((c2 & 0xC0) != 0x80)
			return false;
		
		//c3 deve essere di tipo 0x10xxxxxx
		char c3 = src[3];
		if ((c3 & 0xC0) != 0x80)
			return false;
		return true;
	}

	return false;
}

//***************************************************************
bool utf8::examineSequence (const char *src, u32 nBytesToCheck, u32 *outNumValidBytes, u32 *outNumValidUTF8Char)
{
	assert (src != NULL && nBytesToCheck>0);

	u32 nValidBytes = 0;
	u32 nValidUTF8 = 0;
	bool ret = true;
	while (nBytesToCheck && src[0] != 0)
	{
		u8 n;
		if (!extractOneUTF8Char (src, nBytesToCheck, &n))
		{
			ret = false;
			break;
		}

		nValidBytes += n;
		nBytesToCheck -= n;
		src += n;
		++nValidUTF8;
	}

	if (outNumValidBytes)
		*outNumValidBytes = nValidBytes;
	if (outNumValidUTF8Char)
		*outNumValidUTF8Char = nValidUTF8;
	return ret;
}

//***************************************************************
bool utf8::findCharOffset (const char *src, u32 nBytesToCheck, u32 iEsimoChar, u32 *outOffsetInByte, u32 *outNumUTF8Char)
{
	assert (NULL != src && nBytesToCheck>0);

	u32 nValidBytes = 0;
	u32 nValidUTF8 = 0;
	bool ret = true;

	while (nBytesToCheck)
	{
		u8 n;
		if (!extractOneUTF8Char (src, nBytesToCheck, &n))
		{
			ret = false;
			break;
		}

		++nValidUTF8;
		if (iEsimoChar-- == 0)
			break;

		nBytesToCheck -= n;
		nValidBytes += n;
		src += n;
	}

	*outOffsetInByte = nValidBytes;
	if (outNumUTF8Char)
		*outNumUTF8Char = nValidUTF8;
	return ret;
}

//***************************************************************
bool utf8::findCharOffsetRev (const char *src, u32 nBytesToCheck, u32 iEsimoChar, u32 *outOffsetInByte, u32 *outNumUTF8Char)
{
	assert (NULL != src && nBytesToCheck>0);

	u32 nValidBytes = 0;
	u32 nValidUTF8 = 0;
	bool ret = true;

	while (nBytesToCheck)
	{
		u8 n;
		if (!extractOneUTF8CharRev (src, nBytesToCheck, &n))
		{
			ret = false;
			break;
		}

		nBytesToCheck-=n;
		nValidBytes += n;
		++nValidUTF8;

		if (iEsimoChar-- == 0)
			break;
	}

	*outOffsetInByte = nBytesToCheck - nValidBytes;
	if (outNumUTF8Char)
		*outNumUTF8Char = nValidUTF8;
	return ret;
}



