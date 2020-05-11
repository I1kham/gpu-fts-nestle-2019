#include "rheaString.h"
#include "rheaMemory.h"

using namespace rhea;

/*********************************************************
 *
 */
void string::trimR (char *s)
{
    if (NULL == s)
        return;

    if (s[0] == 0x00)
        return;

    size_t n = strlen(s);
    --n;
    while (s[n] == ' ')
        s[n--] = 0;
}

/*********************************************************
 *
 */
char* string::alloc (rhea::Allocator *allocator, const char *src)
{
    if (NULL == src)
        return NULL;

    if (src[0] == 0x00)
        return NULL;

    size_t n = strlen(src);
    char *ret = (char*)RHEAALLOC(allocator, n+1);
    memcpy (ret, src, n);
    ret[n]=0;
    return ret;
}

/*********************************************************
 *
 */
void string::copy_s (char *dst, size_t sizeOfDest, const char *src)
{
    assert(dst != NULL);
    assert(sizeOfDest>0);

    dst[0] = 0x00;
    if (NULL == src)
        return;

    if (src[0] == 0x00)
        return;

    size_t srcLen = strlen(src);
    if (srcLen > sizeOfDest -1)
        srcLen = sizeOfDest -1;
    memcpy (dst, src, srcLen);
    dst[srcLen] = 0x00;
}


//*********************************************************
void string::append(char *dst, size_t sizeOfDest, const char *src)
{
	strcat_s(dst, sizeOfDest, src);
}

//*********************************************************
void string::append(char *dst, size_t sizeOfDest, u32 num, u8 minNumOfDigit)
{ 
	char s[16];
	if (minNumOfDigit==0)	
		sprintf_s(s, sizeof(s), "%d", num); 
	else
		sprintf_s(s, sizeof(s), "%0*d", minNumOfDigit, num);
	
	string::append(dst, sizeOfDest, s); 
}

//*********************************************************
void string::append(char *dst, size_t sizeOfDest, i32 num, u8 minNumOfDigit)
{
	char s[16];
	if (minNumOfDigit == 0)
		sprintf_s(s, sizeof(s), "%d", num);
	else
		sprintf_s(s, sizeof(s), "%0*d", minNumOfDigit, num);

	string::append(dst, sizeOfDest, s);
}

//*********************************************************
void string::append(char *dst, size_t sizeOfDest, char c)
{
	char s[2];
	s[0] = c;
	s[1] = 0;
	string::append(dst, sizeOfDest, s);
}
