#include "rheaString.h"


using namespace rhea;
using namespace rhea::string;


//*******************************************************************
bool convert_priv_HexDigitToInt (char hex, u32 *out)
{
	if (hex >='0' && hex<='9')
	{
		*out = (u32)(hex - '0');
		return true;
	}
	
	if (hex >='A' && hex<='F')
	{
		hex -= 'A';
		hex += 'a';
	}

	if (hex>='a' && hex<='f')
	{
		hex -='a';
		hex += 10;
		*out = (u32)hex;
		return true;
	}

	return false;
}

//*******************************************************************
bool convert::hexToInt (const char *s, u32 *out, u32 nBytes)
{
	assert (NULL != s && nBytes>0);

	u32 len;
	if (nBytes != u32MAX)
		len = nBytes;
	else
	{
		len = 0;
		while (s[len] != 0)
			++len;
	}


	--len;
	if (!convert_priv_HexDigitToInt (s[len], out))
		return false;

	u32 mul = 16;
	while (len--)
	{
		u32 n;
		if (!convert_priv_HexDigitToInt (s[len], &n))
			return false;
		*out += n*mul;
		mul <<= 4;
	}

	return true;
}


//*******************************************************************
u32 convert::hash (const char *str)
{
	u32 hash = 0;
	u32 i=0;
	while (str[i])
	{
		u32 c = str[i++];
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

//*******************************************************************
f32 convert::toF32  (const char *s, u32 lenOfS)
{ 
	if (NULL==s || lenOfS==0) 
		return 0; 

    if (lenOfS == u32MAX)
		return (f32)atof(s); 

	char temp[64];
	if (lenOfS > 63)
	{
        DBGBREAK;
		lenOfS = 62;
	}
	memcpy (temp, s, lenOfS);
	temp[lenOfS] = 0;
	return (f32)atof(temp); 
}
