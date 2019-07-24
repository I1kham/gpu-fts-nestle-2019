#include "rheaString.h"

using namespace rhea;
using namespace rhea::string;

//**************************************************
void parser::Iter::setup (const char *src, u32 firstByte, u32 numByteToCheck)
{
	s = src;
	iFirst = iNow = (i32)firstByte;

	if (numByteToCheck == 0)
	{
		iLast = iFirst - 1;
		return;
	}

	if (numByteToCheck != u32MAX)
		iLast = iFirst + (i32)numByteToCheck -1;
	else
		iLast = (i32)strlen(src) -1;
}
