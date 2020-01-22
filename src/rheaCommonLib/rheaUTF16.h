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
		u8		oneUTF16toUTF32(const u16 *utf16Sequence, u32 n_u16_in_src, u32 *out_utf32);
					/*	[utf16Sequence] punta ad una stringa codificata in UTF16 e composta da almeno [n_u16_in_src] u16.
						Estrae il primo valido UTF16 della sequenza, e ritorna il numero di u16 utilizzati per la decodifica.
						Mette inoltre in [out_utf32] la versione UTF32 del carattere UTF16.
						Se ritorna 0, vuol dire che la codifica UTF16 in input era errata
					*/

		u8		oneUTF32toUTF16(u32 utf32, u16 *out, u8 num_of_u16_in_out);
					/* converte il carattere epresso in UTF32 in una sequenza UTF16.
						Potrebbero essere necessari fino a 2 U16 per ospitare la conversione.
						La fn ritorna 0 se [num_of_u16_in_out] non è suff a contenere l'intera codifica.
						In caso di successo, ritorna il num di U16 utilizzati per la conversione
					*/

	}; //namespace utf16
}; //namespace rhea
#endif //_rheaUTF8_h_
