#ifndef _rheaUtils_h_
#define _rheaUtils_h_
#include "rheaDataTypes.h"

namespace rhea
{
    size_t      base64_howManyBytesNeededForEncoding (size_t sizeInBytesOfBufferToEncode);
                /* ritorna il numero di bytes necessari a contenere la rappresentazione
                 * in base64 di dei primi [sizeInBytesOfIn] di [in]
                 */

    bool        base64_encode (void *out, size_t sizeOfOutInBytes, const void *in, size_t sizeInBytesOfIn);
                /* prende i primi [sizeInBytesOfIn] di [in] e li converte nella rappresentazione in base64.
                 * Mette il risultato in out appendendo un 0x00 a fine buffer.
                 *
                 * Ritorna false se [sizeOfOutInBytes] non è suff ad ospitare la conversione
                 */

    int         base64_decode (u8 *binary, size_t *binary_length, const char *base64IN, size_t sizeInBytesOfbase64IN);

    bool        sha1 (void *out, size_t sizeOfOutInBytes, const void *in, size_t sizeInBytesOfIn);
                /* Dato un buffer [in], mette in [out] un hash di 20 byte secondo l'algoritmo sha1
                 *
                 * out deve essere di almeno 20 bytes
                 */


} //namespace rhea

#endif // _rheaUtils_h_

