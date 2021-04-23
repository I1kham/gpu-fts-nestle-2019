#include "rheaUTF8String.h"
#include "../rhea.h"

using namespace rhea;
using namespace rhea::utf8;

//*******************************************
void String::priv_constructor()
{
	allocator = NULL;
	buffer = NULL;
	allocatedSize = 0;
	curSize = 0;
}

//*******************************************
String::String (const String &b)				{ priv_constructor(); setAllocator(rhea::getScrapAllocator()); append(b); }
String::String(const u8* s)				{ priv_constructor(); setAllocator(rhea::getScrapAllocator()); append(s); }
String::String(const char* s)				{ priv_constructor(); setAllocator(rhea::getScrapAllocator()); append(s); }


//*******************************************
String::~String()
{
	if (allocator && buffer)
	{
		RHEAFREE(allocator, buffer);
	}
}

//*******************************************
void String::setAllocator (Allocator *allocIN)
{
	assert (NULL == buffer);
	allocator = allocIN;
}

//*******************************************
void String::prealloc (u32 newSizeInByte)
{
	if (newSizeInByte <= allocatedSize)
		return;

	allocatedSize = newSizeInByte;
	if (NULL == buffer)
	{
		if (NULL == allocator)
			allocator =  rhea::getSysHeapAllocator();
		buffer = RHEAALLOCT(u8*,allocator, allocatedSize);
		buffer[0] = 0;
	}
	else
	{
		u8 *newstr = RHEAALLOCT(u8*,allocator, allocatedSize);
		if (curSize)
			memcpy (newstr, buffer, curSize);
		newstr[curSize] = 0;
		RHEAFREE(allocator, buffer);
		buffer = newstr;
	}
}

//*******************************************
void String::append (const String &b, u32 lenInByte)
{
    if (u32MAX == lenInByte)
        append(b.buffer, b.curSize);
    else
        append(b.buffer, lenInByte);
}

//*******************************************
void String::append (const u8 *b, u32 lenInBytes)
{
	if (lenInBytes == 0)
		return;
	if (NULL == b)
		return;
	if (u32MAX == lenInBytes)
		lenInBytes = string::utf8::lengthInBytes(b);
	if (0 == lenInBytes)
		return;

	prealloc (curSize + lenInBytes + 1);
	memcpy (&(buffer[curSize]), b, lenInBytes);
	curSize += lenInBytes;
	buffer[curSize] = 0;
}

//*******************************************
i32 String::findFirst (const rhea::UTF8Char &ch, u32 startIndex) const
{
	if (startIndex >= curSize)
		return -1;

	string::utf8::Iter iter;
	iter.setup (buffer, startIndex, curSize - startIndex);

	UTF8Char c;
	while (!(c = iter.getCurChar()).isEOF())
	{
		if (c == ch)
			return (i32)(startIndex + iter.getCursorPos());
		iter.advanceOneChar();
	}
	return -1;
}

//*******************************************
bool String::isEqualTo (const String &b, bool bCaseSensitive) const
{
	if (curSize != b.curSize)
		return false;
	if (curSize == 0)
		return true;
	return string::utf8::areEqualWithLen (buffer, b.buffer, bCaseSensitive, curSize);
}

//*******************************************
bool String::isEqualTo (const u8* b, bool bCaseSensitive) const
{
	if (curSize == 0)
	{
		if (NULL == b || b[0] == 0x00)
			return true;
		return false;
	}

	if (NULL == b || b[0] == 0x00)
		return false;
	
	const u32 lenB = string::utf8::lengthInBytes (b);
	if (lenB != curSize)
		return false;
	return string::utf8::areEqualWithLen (buffer, b, bCaseSensitive, curSize);
}

//*******************************************
bool String::isEqualToWithLen (const String &b, u32 lenInBytes, bool bCaseSensitive) const
{
	if (curSize < lenInBytes)
		return false;
	if (b.curSize < lenInBytes)
		return false;
	return string::utf8::areEqualWithLen (buffer, b.buffer, bCaseSensitive, lenInBytes);
}

//*******************************************
bool String::isEqualToWithLen (const u8* b, u32 lenInBytes, bool bCaseSensitive) const
{
	if (curSize < lenInBytes)
		return false;

	const u32 lenB = string::utf8::lengthInBytes (b);
	if (lenB < lenInBytes)
		return false;
	
	return string::utf8::areEqualWithLen (buffer, b, bCaseSensitive, lenInBytes);
}

//*******************************
u32 String::explode (const UTF8Char &cTofind, Array<String> &out) const
{
	if (lengthInBytes() == 0)
		return 0;

	u32 nStartElem = out.getNElem();
	u32 nFound = 0;
	u32 iStartByte = 0;
	
	string::utf8::Iter src;
	src.setup (buffer, 0, lengthInBytes());
	
	while (string::utf8::advanceUntil (src, &cTofind, 1))
	{
		out[nStartElem++].setFrom (&buffer[iStartByte], src.getCursorPos() - iStartByte);
		++nFound;

		src.advanceOneChar();
		iStartByte = src.getCursorPos();
	}
	

	if (iStartByte < lengthInBytes())
	{
		const u32 bytesToCopy = lengthInBytes() - iStartByte;
		out[nStartElem].setFrom (&buffer[iStartByte], bytesToCopy);
		++nFound;
	}

	return nFound;
}

//*******************************
void String::trimL()
{
	if (lengthInBytes() == 0)
		return;

	u32 i = 0;
	while (buffer[i]==' ')
		++i;
	if (buffer[i]==0x00)
		clear();
	else
	{
		if (i>0)
		{
			memmove (buffer, &buffer[i], curSize -i);
			curSize -= i;
			buffer[curSize] = 0;
		}
	}
}

//*******************************
void String::trimR()
{
	if (lengthInBytes() == 0)
		return;

	u32 i = curSize -1;
	while (i>0 && buffer[i]==' ')
		--i;
	if (buffer[i]==' ')
		curSize = i;
	else
		curSize = i+1;
	buffer[curSize] = 0;
}

//*******************************
void String::sanitizePath()
{
	if (curSize == 0)
		return;
	rhea::fs::sanitizePathInPlace (this->buffer, curSize);
	curSize = rhea::string::utf8::lengthInBytes(this->buffer);	
}