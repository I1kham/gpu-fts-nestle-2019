#ifndef _rheaUTF16_h_
#define _rheaTF16_h_
#include <stdlib.h>
#include <string.h>
#include "OS/OS.h"
#include "rheaDataTypes.h"


namespace rhea
{
	namespace utf16
	{
		u8			oneUTF16toUTF32(const u16 *utf16Sequence, u32 n_u16_in_src, u32 *out_utf32);
					/*	[utf16Sequence] punta ad una stringa codificata in UTF16 e composta da almeno [n_u16_in_src] u16.
						Estrae il primo valido UTF16 della sequenza, e ritorna il numero di u16 utilizzati per la decodifica.
						Mette inoltre in [out_utf32] la versione UTF32 del carattere UTF16.
						Se ritorna 0, vuol dire che la codifica UTF16 in input era errata
					*/

		u8			oneUTF32toUTF16(u32 utf32, u16 *out, u8 num_of_u16_in_out);
					/* converte il carattere epresso in UTF32 in una sequenza UTF16.
						Potrebbero essere necessari fino a 2 U16 per ospitare la conversione.
						La fn ritorna 0 se [num_of_u16_in_out] non è suff a contenere l'intera codifica.
						In caso di successo, ritorna il num di U16 utilizzati per la conversione
					*/

		bool		toUTF8 (const u16* const src, u8 *out_utf8, u32 *in_out_sizeofOutUTF8);
					/*	[in_out_sizeofOutUTF8] in input contiene la dimensione in bytes di [out_uft8].
						In output, contiene il num di bytes di [out_utf8] che sono stati utilizzati.
						Ritorna true se la conversione ha avuto successo.
						Se ritorna false, ci sono buone probabilità che il problema sia la scarsa lunghezza di [out_utf8], ovvero non c'erano abbastanza bytes
						per completare la conversione.
						In ogni caso, all'uscita della fn [in_out_sizeofOutUTF8] contiene il numero di bytes necessari alla conversione, comprensivi di 0x00 finale.
						E' possibile anche passare NULL a [out_utf8].
						In questo caso la fn ritorna true, e mette in [in_out_sizeofOutUTF8] il num di bytes necessari alla conversione
					*/


		u32			length (const u16 *s);
					/* ritorna il numero di caratteri della stringa (non il numero di bytes). NB che un caratter 
					*/

		u32			rtrim(u16 *s);
					/* trimma a destra e ritorna la nuova lunghezza della string trimmata() */

		void		concatFromASCII(u16 *dst, u32 sizeofDstInBytes, const char* const src);
					/* appende la stringa ASCII [src] alla string utf16 [dst] */

		void		prepend(u16 *dst, u32 sizeOfDstInBytes, const u16* const strToPrepend);
					/* prepende [strToPrepend] a [dst]
					*/

		u32			utf16SequenceToU8Buffer_LSB_MSB (const u16 *utf16_seq, u8 *out_buffer, u32 sizeofOutBuffer, bool bInclude0x00);
					/* [utf16_seq] è una valida sequenza di utf16, terminante con 0x0000
						Prende ogni singolo u16 di [utf16_seq] e lo mette in out_buffer nel formato Little Endian.
						Ritorna il numero di byte utilizzati di out_buffer.
						Se [bInclude0x00] == true, mette anche un singolo byte 0x00 alla fine di [out_buffer]
					*/
		
		

	}; //namespace utf16
}; //namespace rhea
#endif //_rheaUTF8_h_
