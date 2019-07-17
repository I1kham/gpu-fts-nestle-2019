#include "rheaString.h"
#include "rheaAllocator.h"


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
    char *ret = (char*)allocator->alloc (n+1, 8);
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

