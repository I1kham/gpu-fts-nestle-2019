#include "rhea.h"
#include "rheaBit.h"

using namespace rhea;

//***************************************************************
void bit::zero	(void *buffer, u32 bufferLenInByte)
{
    assert (buffer && bufferLenInByte);
    memset (buffer, 0, bufferLenInByte);
}

//***************************************************************
void bit::set (void *buffer, u32 bufferLenInByte, u32 bitToSet)
{
    const u32	byte = bitToSet / 8;
    const u32	bit  = bitToSet % 8;

    assert (buffer && bufferLenInByte && byte < bufferLenInByte);

    //voglio settare i bit da sx a dx, per cui il bit 0 è il 0x80 del primo byte, il bit 1 è il 0x40 del primo byte, il bit 8 è il 0x80 del secondo byte
    const u8	mask = 0x80 >> bit;

    u8 *p = (u8*)buffer;
    p[byte] |= mask;
}

//***************************************************************
void bit::unset	(void *buffer, u32 bufferLenInByte, u32 bitToUnSet)
{
    const u32	byte = bitToUnSet / 8;
    const u32	bit  = bitToUnSet % 8;

    assert (buffer && bufferLenInByte && byte < bufferLenInByte);

    //voglio settare i bit da sx a dx, per cui il bit 0 è il 0x80 del primo byte, il bit 1 è il 0x40 del primo byte, il bit 8 è il 0x80 del secondo byte
    const u8	mask = 0x80 >> bit;

    u8 *p = (u8*)buffer;
    p[byte] &= (~mask);
}

//***************************************************************
bool bit::isSet	(const void *buffer, u32 bufferLenInByte, u32 bitIN)
{
    const u32	byte = bitIN / 8;
    const u32	bit  = bitIN % 8;

    assert (buffer && bufferLenInByte && byte < bufferLenInByte);

    //voglio settare i bit da sx a dx, per cui il bit 0 è il 0x80 del primo byte, il bit 1 è il 0x40 del primo byte, il bit 8 è il 0x80 del secondo byte
    const u8	mask = 0x80 >> bit;

    u8 *p = (u8*)buffer;
    return ((p[byte] & mask) != 0);
}

//***************************************************************
void bit::set (void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToSet)
{
    u32	byte = bitStart / 8;
    u32	bit  = bitStart % 8;
    u8 *p = (u8*)buffer;
    assert (buffer && bufferLenInByte && byte < bufferLenInByte);

    //fillo il primo byte
    /*
    0 1 2 3 4 5 6 7    0 1 2 3 4 5 6 7
    0 0 0 0 0 0 0 0    0 0 0 0 0 0 0 0
    */
    if (bit != 0)
    {
        u8 mask = 0;
        while (bit != 8 && nBitToSet)
        {
            mask |= (0x80 >> bit);
            ++bit;
            --nBitToSet;
        }
        p[byte] |= mask;
        ++byte;
    }

    //a questo punto devo fillare N byte pieni
    const u32 nByteToFill = nBitToSet / 8;
    if (nByteToFill)
    {
        assert (byte + nByteToFill <= bufferLenInByte);
        memset (&p[byte], 0xff, nByteToFill);
        nBitToSet -= nByteToFill * 8;
        byte += nByteToFill;
    }

    //arrivati qui devo fillare i primi nBit
    if (nBitToSet)
    {
        assert (byte < bufferLenInByte && nBitToSet < 8);
        u32 mask = 0x80;
        for (u32 i=1; i<nBitToSet; i++)
            mask |= (0x80 >> i);
        p[byte] |= mask;
    }
}

//***************************************************************
void bit::unset (void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToUnset)
{
    u32	byte = bitStart / 8;
    u32	bit  = bitStart % 8;
    u8 *p = (u8*)buffer;
    assert (buffer && bufferLenInByte && byte < bufferLenInByte);

    //fillo il primo byte
    /*
    0 1 2 3 4 5 6 7    0 1 2 3 4 5 6 7
    0 0 0 0 0 0 0 0    0 0 0 0 0 0 0 0
    */
    if (bit != 0)
    {
        u8 mask = 0;
        while (bit != 8 && nBitToUnset)
        {
            mask |= (0x80 >> bit);
            ++bit;
            --nBitToUnset;
        }
        p[byte] &= (~mask);
        ++byte;
    }

    //a questo punto devo fillare N byte pieni
    const u32 nByteToFill = nBitToUnset / 8;
    if (nByteToFill)
    {
        assert (byte + nByteToFill <= bufferLenInByte);
        memset (&p[byte], 0, nByteToFill);
        nBitToUnset -= nByteToFill * 8;
        byte += nByteToFill;
    }

    //arrivati qui devo fillare i primi nBit
    if (nBitToUnset)
    {
        assert (byte < bufferLenInByte && nBitToUnset < 8);
        u32 mask = 0x80;
        for (u32 i=1; i<nBitToUnset; i++)
            mask |= (0x80 >> i);
        p[byte] &= ~mask;
    }
}

//***************************************************************
void bit::get (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToGet, void *out, u32 sizeOfOutInByte)
{
#ifdef _DEBUG
    u32 debug_nByteNeeded = nBitToGet/8;
    if (debug_nByteNeeded * 8 < nBitToGet)
        ++debug_nByteNeeded;
    assert (sizeOfOutInByte >= debug_nByteNeeded);
#endif

    /*
    01234567 01234567 01234567
       xxxxx yyyyyyyy zz

    01234567 01234567 01234567 01234567
    ---xxxxx yyyyyyyy zz------ --------

    01234567 01234567 01234567
    xxxxx000 yyyyyyyy zz------

    01234567 01234567 01234567
    xxxxxyyy yyyyy000 zz------

    const u32	byteStart = bitStart / 8;
    const u32	byteEnd   = (bitStart + nBitToGet-1) / 8;
    const u32	bytesToCopy = (byteEnd - byteStart) +1;


    assert (bytesToCopy && sizeOfOutInByte >= bytesToCopy);
    u8 *p = (u8*)buffer;
    memcpy (out, &p[byteStart], bytesToCopy);

    //setto a zero i bit dell'ultimo byte che non mi interessano
    const u32 nUsefullLastBit = (bitStart + nBitToGet) % 8;
    if (nUsefullLastBit)
    {
        u8 *p = (u8*)out;
        bit::unset (&p[bytesToCopy-1], 1, nUsefullLastBit, 8-nUsefullLastBit);
    }

    //ora devo shiftare out di nbit
    const u32 shift = bitStart % 8;
    const u32 carry_shift = 8 - shift;
    if (shift)
    {
        u8 *p = (u8*)out;

        p[0] <<= shift;
        for (u32 i=1; i<bytesToCopy; i++)
        {
            const u8 b = (p[i] << shift);
            const u8 carry = (p[i] >> carry_shift);
            p[i-1] |= carry;
            p[i] = b;
        }
    }
    */


    /*
    01234567 01234567 01234567
       xxxxx yyyyyyyy zzz


    firstByte = xxxxx000

    01234567 01234567
    yyyyyyyy zzz-----

        01234567
    lc= 00000000
    b=  00000yyy
    c=  yyyyy000

        01234567
    lc= yyyyy000
    b=  00000zzz
    c=  -----000


    01234567 01234567
    00000yyy yyyyyzzz


    ///////////////////////////////////
    01234567 01234567 01234567
    ---xxxxx yyyyyyyy zz------

    01234567 01234567
    yyyyyyyy zz------

        01234567
    lc= 00000000
    b=  00000yyy
    c=  yyyyy000

        01234567
    lc= yyyyy000
    b=  00000zz-
    c=  -----000


    01234567 01234567
    00000yyy yyyyyzz-
    */

    if (bitStart % 8 == 0)
    {
        //caso fortunato, possiamo andare di memcpy
        u32	bytesToCopy = nBitToGet / 8;
        if (bytesToCopy * 8 < nBitToGet)
            ++bytesToCopy;

        assert (bytesToCopy && sizeOfOutInByte >= bytesToCopy);
        u8 *pBuffer = (u8*)buffer;
        memcpy (out, &pBuffer[(bitStart / 8)], bytesToCopy);

        //devo azzerare gli ultimi bit se non servono
        const u32 b = (nBitToGet % 8);
        if (b)
        {
            u8 mask = 0;
            for (u32 i=0; i<b; i++)
                mask |= (0x80 >> i);
            ((u8*)out)[bytesToCopy-1] &= mask;
        }
    }
    else if (nBitToGet <= 8)
    {
        //siamo nel caso in cui s'ha da copiare qualche bit (<8), vado di brute force
        u8 *pOUT = (u8*)out;
        pOUT[0] = 0;

        u8 *p = (u8*)buffer;
        u32	byte = bitStart / 8;
        u32	bit  = bitStart % 8;
        u8 maskOUT = 0x80;
        while (nBitToGet--)
        {
            const u8 mask = 0x80 >> bit;
            if ((p[byte] & mask) != 0)
                pOUT[0] |= maskOUT;
            maskOUT>>=1;
            ++bit;
            if (bit == 8)
            {
                ++byte;
                bit = 0;
            }
        }
    }
    else
    {
        const u32	byteStart = (bitStart / 8) + 1;
        const u32	byteEnd   = (bitStart + nBitToGet-1) / 8;
        const u32	bytesToCopy = (byteEnd - byteStart) +1;

        assert (bytesToCopy && sizeOfOutInByte >= bytesToCopy);
        u8 *pBuffer = (u8*)buffer;
        memcpy (out, &pBuffer[byteStart], bytesToCopy);

        //setto a zero i bit dell'ultimo byte che non mi interessano
        const u32 nUsefullLastBit = (bitStart + nBitToGet) % 8;
        if (nUsefullLastBit)
        {
            u8 *pOUT = (u8*)out;
            bit::unset (&pOUT[bytesToCopy-1], 1, nUsefullLastBit, 8-nUsefullLastBit);
        }

        //ora devo shiftare out di nbit verso destra e alla fine inserire il firstByte
        const u32 carry_shift = bitStart % 8;
        const u32 shift = 8 - carry_shift;
        if (carry_shift)
        {
            u8 *pOUT = (u8*)out;

            u8 last_carry = 0;
            for (u32 i=0; i<bytesToCopy; i++)
            {
                const u8 b = (pOUT[i] >> shift);
                const u8 carry = (pOUT[i] << carry_shift);
                pOUT[i] = b | last_carry;
                last_carry = carry;
            }

            if (nBitToGet % 8)
            {
                assert (bytesToCopy < sizeOfOutInByte);
                pOUT[bytesToCopy] = last_carry;
            }

            //setto il primo byte
            const u8 b = pBuffer[byteStart-1] << carry_shift;
            pOUT[0] |= b;
        }
    }

}

//***************************************************************
void bit::write	(void *bufferOUT, u32 bufferOUTLenInByte, u32 bitStartOUT, const void *bufferIN, u32 bufferINLenInByte, u32 bitStartIN, u32 nBitToWrite)
{
    u32 byteR = bitStartIN / 8;
    u32 bitR  = bitStartIN % 8;
    u32 byteW = bitStartOUT / 8;
    u32 bitW  = bitStartOUT % 8;

    assert (bufferINLenInByte*8 >= bitStartIN + nBitToWrite);
    assert (bufferOUTLenInByte*8 >= bitStartOUT + nBitToWrite);

    const u8 *in = (u8*)bufferIN;
    u8 *out = (u8*)bufferOUT;
    while (nBitToWrite--)
    {
        const u8 maskR = 0x80 >> bitR;
        const u8 maskW = 0x80 >> bitW;
        (in[byteR] & maskR) == 0 ? out[byteW] &= (~maskW) : out[byteW] |= maskW;

        if (++bitR == 8)	{ bitR=0; ++byteR; }
        if (++bitW == 8)	{ bitW=0; ++byteW; }
    }
}

//***************************************************************
template <class T>
void internal_writeValue (void *buffer, u32 bufferLenInByte, u32 bitStart, T valueToWrite)
{
    const u8 NBIT = 8*sizeof(T);
    u32 byteW = bitStart / 8;
    u32 bitW  = bitStart % 8;

    assert (bufferLenInByte*8 >= bitStart + NBIT);
    u8 *out = (u8*)buffer;
    T maskR = 0x01 << (NBIT -1);
    for (u32 i=0; i<NBIT; i++)
    {
        const u8 maskW = 0x80 >> bitW;
        (valueToWrite & maskR) == 0 ? out[byteW] &= (~maskW) : out[byteW] |= maskW;

        maskR>>=1;
        if (++bitW == 8)	{ bitW=0; ++byteW; }
    }
}


//***************************************************************
template <class T>
T internal_readValue (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    const u8 NBIT = 8*sizeof(T);
    const T  maskW = 0x01 << (NBIT -1);
    T ret = 0;

    u32 byteR = bitStart / 8;
    u32 bitR  = bitStart % 8;

    assert (bufferLenInByte*8 >= bitStart + NBIT);
    const u8 *in = (u8*)buffer;
    for (u32 i=0; i<NBIT; i++)
    {
        const u8 maskR = (0x80 >> bitR);
        if ((in[byteR] & maskR) != 0)
            ret |= (maskW >> i);

        if (++bitR == 8)	{ bitR=0; ++byteR; }
    }

    return ret;
}


//***************************************************************
void bit::writeU32 (void *buffer, u32 bufferLenInByte, u32 bitStart, u32 valueToWrite)
{
    internal_writeValue<u32> (buffer, bufferLenInByte, bitStart, valueToWrite);
}

u32 bit::readU32 (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    return internal_readValue<u32> (buffer, bufferLenInByte, bitStart);
}

//***************************************************************
void bit::writeU16 (void *buffer, u32 bufferLenInByte, u32 bitStart, u16 valueToWrite)
{
    internal_writeValue<u16> (buffer, bufferLenInByte, bitStart, valueToWrite);
}

u16 bit::readU16 (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    return internal_readValue<u16> (buffer, bufferLenInByte, bitStart);
}


//***************************************************************
void bit::writeU8 (void *buffer, u32 bufferLenInByte, u32 bitStart, u8 valueToWrite)
{
    internal_writeValue<u8> (buffer, bufferLenInByte, bitStart, valueToWrite);
}

u8	bit::readU8 (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    return internal_readValue<u8> (buffer, bufferLenInByte, bitStart);
}


//***************************************************************
u32 bit::findFirstZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    const u32 NBIT = bufferLenInByte * 8;
    assert (bitStart < NBIT);
    u32 byteR = bitStart / 8;
    u32 bitR  = bitStart % 8;
    u8 *p = (u8*)buffer;

    //i primi bit li controllo uno per uno
    if (bitR > 0)
    {
        while (bitR < 8)
        {
            const u8 mask = 0x80 >> bitR;
            if ((p[byteR] & mask) == 0)
                return bitStart;
            ++bitStart;
            ++bitR;
        }
        bitR = 0;
        ++byteR;
    }

    //ora sono su un bit=0 il che vuol dire che posso controllare un byte alla volta
    u32 nbitToCheck = NBIT - bitStart;
    assert (nbitToCheck % 8 == 0);
    while (nbitToCheck)
    {
        assert (byteR < bufferLenInByte);
        const u8 b = p[byteR];
        if (b == 0xFF)
        {
            ++byteR;
            nbitToCheck -= 8;
            continue;
        }

        //ho trovato un byte con dei bit a zero, cerco il primo
        for (u8 i=0; i<8; i++)
        {
            const u8 mask = 0x80 >> i;
            if ((b & mask) == 0)
                return byteR*8 + i;
        }

        //qui non ci devo mai arrivare!
        DBGBREAK;
        return u32MAX;
    }

    return u32MAX;
}

//***************************************************************
u32 bit::findFirstONE (const void *buffer, u32 bufferLenInByte, u32 bitStart)
{
    const u32 NBIT = bufferLenInByte * 8;
    assert (bitStart < NBIT);
    u32 byteR = bitStart / 8;
    u32 bitR  = bitStart % 8;
    u8 *p = (u8*)buffer;

    //i primi bit li controllo uno per uno
    if (bitR > 0)
    {
        while (bitR < 8)
        {
            const u8 mask = 0x80 >> bitR;
            if ((p[byteR] & mask) != 0)
                return bitStart;
            ++bitStart;
            ++bitR;
        }
        bitR = 0;
        ++byteR;
    }

    //ora sono su un bit=0 il che vuol dire che posso controllare un byte alla volta
    u32 nbitToCheck = NBIT - bitStart;
    assert (nbitToCheck % 8 == 0);
    while (nbitToCheck)
    {
        assert (byteR < bufferLenInByte);
        const u8 b = p[byteR];
        if (b == 0)
        {
            ++byteR;
            nbitToCheck -= 8;
            continue;
        }

        //ho trovato un byte con dei bit a zero, cerco il primo
        for (u8 i=0; i<8; i++)
        {
            const u8 mask = 0x80 >> i;
            if ((b & mask) != 0)
                return byteR*8 + i;
        }

        //qui non ci devo mai arrivare!
        DBGBREAK;
        return u32MAX;
    }

    return u32MAX;
}

//***************************************************************
u32 bit::findFirstSequenceOfZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToFind)
{
    if (nBitToFind == 1)
        return findFirstZERO (buffer, bufferLenInByte, bitStart);

    const u32 NBIT_MAX = bufferLenInByte * 8 -nBitToFind;
    while (bitStart < NBIT_MAX)
    {
        bitStart = findFirstZERO (buffer, bufferLenInByte, bitStart);
        if (bitStart == u32MAX || bitStart >= NBIT_MAX)
            return u32MAX;

        u32	b2 = findFirstONE (buffer, bufferLenInByte, bitStart + 1);
        if (b2 - bitStart >= nBitToFind)
            return bitStart;

        bitStart = b2+1;
    }
    return u32MAX;
}

//***************************************************************
u32 bit::findFirstSequenceOfONE (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToFind)
{
    if (nBitToFind == 1)
        return findFirstONE (buffer, bufferLenInByte, bitStart);

    const u32 NBIT_MAX = bufferLenInByte * 8 -nBitToFind;

    while (bitStart < NBIT_MAX)
    {
        bitStart = findFirstONE (buffer, bufferLenInByte, bitStart);
        if (bitStart == u32MAX || bitStart >= NBIT_MAX)
            return u32MAX;

        u32	b2 = findFirstZERO (buffer, bufferLenInByte, bitStart + 1);
        if (b2 - bitStart >= nBitToFind)
            return bitStart;

        bitStart = b2+1;
    }
    return u32MAX;
}

//***************************************************************
bool bit::isSequenceOfZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToCheck)
{
    assert (bitStart + nBitToCheck <= bufferLenInByte * 8);
    u32 byteR = bitStart / 8;
    u32 bitR  = bitStart % 8;
    u8 *p = (u8*)buffer;

    //i primi bit li controllo uno per uno
    if (bitR > 0)
    {
        while (bitR < 8 && nBitToCheck)
        {
            const u8 mask = 0x80 >> bitR;
            if ((p[byteR] & mask) != 0)
                return false;
            ++bitR;
            --nBitToCheck;
        }
        ++byteR;
    }

    //ora sono su un bit=0 il che vuol dire che posso controllare un byte alla volta
    u32 nByteToCheck = nBitToCheck / 8;
    while (nByteToCheck--)
    {
        assert (byteR < bufferLenInByte);
        const u8 b = p[byteR++];
        if (b != 0)
            return false;
    }

    const u32 nBitLeftToCheck = nBitToCheck % 8;
    for (u32 i=0; i<nBitLeftToCheck; i++)
    {
        const u8 mask = 0x80 >> i;
        if ((p[byteR] & mask) != 0)
            return false;
    }
    return true;
}

//***************************************************************
bool bit::isSequenceOfONE (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToCheck)
{
    assert (bitStart + nBitToCheck <= bufferLenInByte * 8);
    u32 byteR = bitStart / 8;
    u32 bitR  = bitStart % 8;
    u8 *p = (u8*)buffer;

    //i primi bit li controllo uno per uno
    if (bitR > 0)
    {
        while (bitR < 8 && nBitToCheck)
        {
            const u8 mask = 0x80 >> bitR;
            if ((p[byteR] & mask) == 0)
                return false;
            ++bitR;
            --nBitToCheck;
        }
        ++byteR;
    }

    //ora sono su un bit=0 il che vuol dire che posso controllare un byte alla volta
    u32 nByteToCheck = nBitToCheck / 8;
    while (nByteToCheck--)
    {
        assert (byteR < bufferLenInByte);
        const u8 b = p[byteR++];
        if (b != 0xFF)
            return false;
    }

    const u32 nBitLeftToCheck = nBitToCheck % 8;
    for (u32 i=0; i<nBitLeftToCheck; i++)
    {
        const u8 mask = 0x80 >> i;
        if ((p[byteR] & mask) == 0)
            return false;
    }
    return true;
}
