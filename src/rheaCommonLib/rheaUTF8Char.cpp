#include "rheaUTF8Char.h"
#include "rheaUTF8.h"
#include "OS/OS.h"

using namespace rhea;
using namespace rhea::utf8;

#ifdef _DEBUG
	//***************************************************
	void Char::assertStringIsComposedByOnlyOneUTF8Char (const u8 *c) const
	{
		u32 numValidBytes, numValidUTF8Char;
		if (!utf8::examineSequence (c, 4, &numValidBytes, &numValidUTF8Char))
			DBGBREAK;
		if (numValidUTF8Char==0 || numValidUTF8Char>1)
			DBGBREAK;
	}
#endif

//***************************************************
void Char::setFromUTF32 (u32 source)
{
	memset (c, 0, 4);
	if (source < 0x00000080) 
	{
		c[0] = (u8)source;
		return;
	}

	// Upper bounds checking.
	if (source > 0x0010FFFF) 
	{
		// Invalid character.  What should we do?
		// Return a default character.
		DBGBREAK;
		c[0] = '?';
		return;
	}

	// Every other case uses bit markers.
	// Start from the lowest encoding and check upwards.
	u32 ui32High = 0x00000800;
	u32 ui32Mask = 0xC0;
	u8 nU8Needed = 2;
	while (source >= ui32High)
	{
		ui32High <<= 5;
		ui32Mask = (ui32Mask >> 1) | 0x80UL;
		++nU8Needed;
	}

	// Encode the first byte.
	u32 ui32BottomMask = ~((ui32Mask >> 1) | 0xFFFFFF80);
	c[0] = (u8) (ui32Mask | ((source >> ((nU8Needed - 1) * 6)) & ui32BottomMask));

	// Now fill in the rest of the bits.
	u8 iNext=1;
	for (u32 I = nU8Needed - 1; I--;) 
	{
		// Shift down, mask off 6 bits, and add the 10xxxxxx flag.
		c[iNext++] = (u8) (((source >> (I * 6)) & 0x3F) | 0x80);
	}
}



//***************************************************
u32 Char::toUTF32 () const
{
	u32 u;
	if (utf8::oneUTF8ToUTF32(c, len(), &u))
		return u;
	return 0;
}

//******************************************************
u8 Char::setFrom (const u8 *src, u32 srclenInByte)
{
	memset (c, 0, 4);
	u8 n;
	if (!utf8::extractOneUTF8Char (src, srclenInByte, &n))
		return 0;

	assert (n<5);
	memcpy (c, src, n);
	return n;
}

//******************************************************
u8 Char::setFromRev (const u8 *src, u32 srclenInByte)
{
	memset (c, 0, 4);
	u8 n;
	if (!utf8::extractOneUTF8CharRev (src, srclenInByte, &n))
		return 0;

	assert (n<5);
	memcpy (c, src, n);
	return n;
}

//*********************************************
bool Char::isEqualNoCaseSensitive (const Char &b) const
{
	//il "no case sensitive" funziona solo per gli ascii
	if (isAscii() && b.isAscii())
	{
		char c1 = c[0];
		if (c1>='A' && c1<='Z')		{ c1 -= 'A'; c1 += 'a'; }
		char c2 = b.c[0];
		if (c2>='A' && c2<='Z')		{ c2 -= 'A'; c2 += 'a'; }
		if (c1 == c2)
			return true;
		return false;
	}

	if (memcmp (c, b.c, 4) == 0)
		return true;
	return false;
}