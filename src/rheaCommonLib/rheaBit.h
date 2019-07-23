#ifndef _rheaBit_h_
#define _rheaBit_h_
#include "rheaDataTypes.h"

namespace rhea
{
    namespace bit
    {
        void	zero	(void *buffer, u32 bufferLenInByte);
        void	set		(void *buffer, u32 bufferLenInByte, u32 bitToSet);
        void	unset	(void *buffer, u32 bufferLenInByte, u32 bitToUnSet);
        bool	isSet	(const void *buffer, u32 bufferLenInByte, u32 bit);

        void	set		(void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToSet);
        void	unset	(void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToUnset);
            //setta/resetta a 1 i primi "nBitToSet" a partire da bitStart compreso

        void	get		(const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToGet, void *out, u32 sizeOfOutInByte);
            /*	prende i primi "nBitToGet" a partire da  da bitStart compreso e li metto in out
                i bit internamente sono memorizzati a partire da 0x80 in giu, quindi il bit 0=0x80, bit1=0x40 e via dicendo.
                Lo stesso schema viene applicato ad out
            */

        void	write	(void *bufferOUT, u32 bufferOUTLenInByte, u32 bitStartOUT,
                         const void *bufferIN, u32 bufferINLenInByte, u32 bitStartIN,
                        u32 nBitToWrite);
                /*
                    legge i primi nBitToWrite di bufferIN a paratire da bitStartIN e li mette
                    in bufferOUT, a partire da bitStartOUT
                */

        void	writeU32 (void *buffer, u32 bufferLenInByte, u32 bitStart, u32 valueToWrite);
        u32		readU32 (const void *buffer, u32 bufferLenInByte, u32 bitStart);
        void	writeU16 (void *buffer, u32 bufferLenInByte, u32 bitStart, u16 valueToWrite);
        u16		readU16 (const void *buffer, u32 bufferLenInByte, u32 bitStart);
        void	writeU8 (void *buffer, u32 bufferLenInByte, u32 bitStart, u8 valueToWrite);
        u8		readU8 (const void *buffer, u32 bufferLenInByte, u32 bitStart);

        u32		findFirstZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart);
        u32		findFirstONE (const void *buffer, u32 bufferLenInByte, u32 bitStart);
                    /*	ritorna il primo bit a zero/uno. La ricerca parte da bitStart
                        ritorn u32MAX se non trova nessun bit a zero/uno
                    */

        u32		findFirstSequenceOfZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToFind);
        u32		findFirstSequenceOfONE (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToFind);
                    /*	cerca una sequenza di nBitToFind tutti a zero/uno e, se la trova, ritorna il bit
                        di partenza della sequenza.
                        La ricerca parte da bitStart
                        ritorn u32MAX se non trova la sequenza
                    */

        bool	isSequenceOfZERO (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToCheck);
        bool	isSequenceOfONE (const void *buffer, u32 bufferLenInByte, u32 bitStart, u32 nBitToCheck);
                    //	ritorna true se i primi nBitToCheck a partire da bitStart (compreso) sono tutti ZERO/ONE

    } //namespace bit

} //namespace rhea
#endif // _rheaBit_h_

