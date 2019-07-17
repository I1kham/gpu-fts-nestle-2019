#include "rheaString.h"
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

using namespace rhea;
using namespace rhea::string;


//*******************************************************************
void format::timeMicroSecToHHMMSSMsUs (u64 t, char *out, u32 numCharInOut)
{
    u32 hh = (u32) (t/3600000000u);
    t -= hh*3600000000u;

	u32 mm = (u32)(t/60000000);
	t -= mm*60000000;

	u32 sec = (u32)(t / 1000000);
	t -= sec*1000000;
	
	u32 ms = (u32)(t / 1000);	
	t -= ms*1000;

	u32 us = (u32)t;

    sprintf_s (out, numCharInOut, "%02d:%02d:%02d:%03d:%03d", hh, mm, sec, ms, us);
}

//*******************************************************************
void format::U64 (u64 val, char thousandSep, char *out, u32 numCharInOut)
{
	assert (NULL!=out && numCharInOut>0);

	char	tempStr[64];
    sprintf_s (tempStr, sizeof(tempStr), "%" PRIu64, val);

	int len = (int)strlen(tempStr);
	int	numSeparatori = (len-1) / 3;
	int	firstSep = len - numSeparatori*3;

	int ct=0;
	int i=0;
	for (; i<firstSep; i++)
		out[ct++] = tempStr[i];

	if (i<len)
	{
		out[ct++] = thousandSep;

		u8 m=0;
		for (; i<len; i++)
		{
			out[ct++] = tempStr[i];
			++m;
			if (m == 3 && i<len-1)
			{
				out[ct++] = thousandSep;
				m=0;
			}
		}
	}
	out[ct] = 0;
}

//*******************************************************************
void format::U32 (u32 val, char thousandSep, char *out, u32 numCharInOut)
{
	assert (NULL!=out && numCharInOut>0);

	char	tempStr[64];
	sprintf_s (tempStr, sizeof(tempStr), "%d", val);

	int len = (int)strlen(tempStr);
	int	numSeparatori = (len-1) / 3;
	int	firstSep = len - numSeparatori*3;

	int ct=0;
	int i=0;
	for (; i<firstSep; i++)
		out[ct++] = tempStr[i];

	if (i<len)
	{
		out[ct++] = thousandSep;

		u8 m=0;
		for (; i<len; i++)
		{
			out[ct++] = tempStr[i];
			++m;
			if (m == 3 && i<len-1)
			{
				out[ct++] = thousandSep;
				m=0;
			}
		}
	}
	out[ct] = 0;
}

//*******************************************************************
void format::F32 (f32 val, u32 numDecimal, char thousandSep, char decimalSep, char *out, u32 numCharInOut)
{
	assert (NULL!=out && numCharInOut>0);

	char	format[8];
	sprintf_s (format, 8, "%%.%df", numDecimal);

	char	tempStr[32];
	sprintf_s (tempStr, 32, format, val);

	i32 n = (i32)strlen(tempStr);
	i32 t = n;
	if (numDecimal > 0)
	{
		if ((u32)n > 4+numDecimal)
		{
			i32 k = n- (numDecimal+1);
			t += (k-1) / 3;
		}
	}
	else
	{
		if (n>3)
			t += (n-1) / 3;
	}
	if ((u32)t>=numCharInOut)
	{
		out[0] = 0;
		DBGBREAK
		return;
	}

	--n;
	out[t--] = 0;
	if (numDecimal > 0)
	{
		while (numDecimal--)
			out[t--] = tempStr[n--];
		out[t--] = decimalSep;
		--n;
	}

	u8 ct = 0;
	while (t>=0)
	{
		if (ct==3)
		{
			ct=0;
			out[t--]  = thousandSep;
		}
		out[t--]  = tempStr[n--];
		++ct;
	}
}


//*******************************************************************
void format::Hex (u32 hex, char *out, u32 sizeofout)
{
	assert (sizeofout > 8);
	u32 mask = 0x0000000F;
	u32 shift = 0;
	u8 i=8;
	while (i--)
	{
		u32 n = (hex & mask) >> shift;
		mask <<= 4;
		shift += 4;

		if (n < 10)
			out[i] = '0' + (char)n;
		else
			out[i] = 'A' +(char)(n-10);
	}
	out[8] = 0;
}
