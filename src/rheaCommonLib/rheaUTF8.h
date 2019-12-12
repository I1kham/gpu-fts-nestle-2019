#ifndef _rheaUTF8_h_
#define _rheaTF8_h_
#include "rheaDataTypes.h"
#include "rheaUTF8Char.h"


namespace rhea
{
	namespace utf8
	{
		bool	isAValidUTF8Char (const char *src , u32 nBytes);
					/* controlla ESATTAMENTE i primi nBytes di src e determina se TUTTI INSIEME corrispondono ad una valida seq utf8
					*/

		u8		detectCharlenInByteByFirstByte (char c);
					/* dato il primo char di una ipotetica seq utf8, ritorna il num di byte necessari per la corretta sequenza */

		bool	examineSequence (const char *src, u32 nBytesToCheck, u32 *outNumValidBytes=NULL, u32 *outNumValidUTF8Char=NULL);
					/*	Se *src contiene una valida seq utf8, allora ritorna true, altrimenti false.
						Anche in caso di false, outNumValidBytes e outNumValidUTF8Char vengono correttamente valorizzati
						riportando il num di bytes buoni (prima di trovare una seq invalida) ed il num di char utf8 contenuti nei bytes buoni
					*/

		bool	extractOneUTF8Char (const char *src, u32 srcLen, u8 *outNumUsedBytes);
					/*	estrae il primo utf8 char da *src, oppure ritorna false se la sequenza è invalida.
						Valorizza outNumUsedBytes con il num di byte corrispondendi al primo utf8 char 
					*/

		bool	extractOneUTF8CharRev (const char *src, u32 srcLen, u8 *outNumUsedBytes);
					/*	parte da src[srcLen-1] e va all'indietro.
						estrae il primo utf8 char, oppure ritorna false se la sequenza è invalida.
						Valorizza outNumUsedBytes con il num di byte corrispondendi al primo utf8 char 
					*/


		bool	findCharOffset (const char *src, u32 nBytesToCheck, u32 iEsimoChar, u32 *outOffsetInByte, u32 *outNumUTF8Char = NULL);
					/*  Avanza fino all i-esimo char.
						Se durante l'avanzamento incontra una sequenza invalida, ritorna false
						Se l'iesimo char non esiste (perchè nBytesToCheck è troppo piccolo), ritorna false.
						In ogni caso, outOffsetInByte viene fillato con la posizione in byte dell'ultimo char buono trovato (o dell'iesimo se la fn ritorna true).
						Lo stesso dicasi per outNumUTF8Char (che pero' è opzionale)
					*/

		bool	findCharOffsetRev (const char *src, u32 nBytesToCheck, u32 iEsimoChar, u32 *outOffsetInByte, u32 *outNumUTF8Char = NULL);
					/*  Parte da src[nBytesToCheck-1] e va all'indietro
						Se durante l'avanzamento incontra una sequenza invalida, ritorna false
						Se l'iesimo char non esiste (perchè nBytesToCheck è troppo piccolo), ritorna false.
						In ogni caso, outOffsetInByte viene fillato con la posizione in byte dell'ultimo char buono trovato (o dell'iesimo se la fn ritorna true).
						Lo stesso dicasi per outNumUTF8Char (che pero' è opzionale)
					*/
						
		/*==============================================================================
		 * parsing delle stringhe
		 *=============================================================================*/
		namespace parser
		{
			extern	utf8::Char CharArray_b_r_n_t[4];	// è un array di comodo che contiene i Char \b \r \n \t


			/*	Buona parte delle fn successive utilizzano Source come input (a volte anche come output). Lo scopo di Source è definire la stringa sulla
				quale le fn devono lavorare, il primo byte da analizzare (iNow) ed un opzionale numero massimo di byte da considerare.
				Le fn che usano Source come input terminano se incontrano 0x00 o se numByteToCheck == 0
				Le fn che usano Source come input, lo modificano aggiornando iNow al prossimo utf8 da scansionare, e aggiornando numByteToCheck in base
				a quanti byte hanno processato.
				Se numByteToCheck==0, curChar() ritorna 0x00, come se avesse raggiunto il fine stringa, e next() ritorna false
				*/
			struct	Source
			{
							Source ()																		{ nullChar.setNULL(); s=NULL; }
							Source (const Source &b)														{ nullChar.setNULL(); copyFrom(b); }
				void		setup (const char *src, u32 firstByte=0, u32 maxByteToChek=u32MAX);
				void		setMaxByteToCheck (u32 i)														{ numByteToCheck=i;}
				u32			getMaxByteToCheck() const														{ return numByteToCheck; }
				bool		next();
				u32			cur() const																		{ return iNow;}
				bool		prev();
				bool		advance (u32 nChar);
				bool		toFirst();
				bool		toLast();
				Source&		operator= (const Source &b)														{ copyFrom(b); return *this; }
				bool		cmp (const char *b, bool bCaseSensitive) const							
							{ 
								if (isEOL()) return false;
								bool ret;
								if (bCaseSensitive) ret = (strncmp (&s[iNow], b, numByteToCheck) == 0); else ret = (_strnicmp (&s[iNow], b, numByteToCheck) == 0); 
								if (ret && b[numByteToCheck]==0) return true;
								return false;				
							}

				const Char&	getCurChar() const																	{ if (s[iNow] == 0x00 || numByteToCheck==0) return nullChar; return curChar; }
				const char*	getCurStrPointer() const															{ if (s[iNow] == 0x00 || numByteToCheck==0) return 0x00; return &s[iNow]; }
				bool		isEOL() const																		{ return getCurStrPointer() == NULL; }
				void		copyCurStr (char *out, u32 sizeofOut) const									
							{
								if (isEOL()) out[0] = 0;
								else { assert (sizeofOut > numByteToCheck); memcpy (out, &s[iNow], numByteToCheck); out[numByteToCheck] = 0; }
							}

			public:
				const char	*s;
				u32			iNow;
				u32			numByteToCheck;
				Char		curChar;

			private:
				void		copyFrom (const Source &b)														{ s=b.s; iNow=b.iNow; numByteToCheck=b.numByteToCheck; curChar=b.curChar; }

			private:
				Char		nullChar;

			};


			inline	bool			isCharMaiuscolo (const Char &c)											{ return c.isCharMaiuscolo(); }
			inline	bool			isCharMinuscolo (const Char &c)											{ return c.isCharMinuscolo(); }
			inline	bool			isANumber (const Char &c)												{ return c.isANumber(); }
			inline	bool			isALetter (const Char &c)												{ return c.isALetter(); }
			inline	bool			isALetterOrANumber (const Char &c)										{ return c.isALetterOrANumber(); }
					bool			IsOneOfThis (const Char &c, const Char *validChars, u32 numOfValidChars);
										// ritorna true se c è uno dei *validChars


					void			skip (Source &src, const Char *toBeskippedChars, u32 numOfToBeskippedChars);
										/*	avanza e si ferma quando trova char != da quelli da skipppare o a fine buffer
											Se trova un char != dai toBeskippedChars, src punta al primo char trovato
										*/

					void			skipBack (Source &src, const Char *toBeskippedChars, u32 numOfToBeskippedChars);
										/*	indietreggia e si ferma quando trova char != da quelli da skipppare o a inizio buffer
											Se trova un char != dai toBeskippedChars, src punta al primo char trovato, oppure a inizio buffer
										*/

					void			skipEOL (Source &src);
										/*	se src punta a \r, allora skippa \r e l'evenutale \n che segue.
											Se src punta a \n, lo skippa
										*/

					void			gotoChar (Source &src, u32 n);
										/* sposta src al char n-esimo o a fine buffer */

					bool			advanceUntil (Source &src, const Char *validTerminators, u32 numOfValidTerminators);
										/*	controlla il char corrente e avanza fino a quando non trova uno dei validTerminators o a fine buffer
											Ritorna true se trova uno dei validTerminators, nel qual caso src punta al primo validTerminators trovato.
											Ritorna false se non trova un validTerminators, nel qual caso src punta a fine buffer
										*/

					bool			backUntil (Source &src, const Char *validTerminators, u32 numOfValidTerminators);
										/*	controlla il char corrente e indietreggia fino a quando non trova uno dei validTerminators o a inizio buffer
											Ritorna true se trova uno dei validTerminators, nel qual caso src punta al primo validTerminators trovato.
											Ritorna false se non trova un validTerminators, nel qual caso src punta a inizio buffer
										*/

					void			advanceToEOL (Source &src, bool bskipEOL=true);
										/*	controllo il char corrente e se è un EOL termina (o lo skippa), altrimenti avanza e ripete il controllo
											All'uscita, src punta a EOL oppure al primo char subito dopo EOL (se bskipEOL=true), oppure a fine buffer
										*/

					bool			advancetoNextWord (Source &src);
					bool			backToPreviousWord (Source &src);
										/*	controlla il char corrente e avanza fino a quando non trova un valido separatore di parole o a fine buffer.
											A quel punto skippa altri eventuali separatori e si ferma sul primo char della prossima parola e ritorna true
											Ritorna false se non trova un valido separatore di parole, nel qual caso src punta a fine buffer
										*/


			inline	void			toNextValidChar (Source &src)											{ parser::skip (src, utf8::parser::CharArray_b_r_n_t, 4); }
										/*	usa la skip() per skippare tutti o "\r\n\t\b" e ritorna l'indice del primo char buono 
										*/


					void			extractLine (Source &src, Source *out_result);
										/*	Prende tutti i caratteri compresti tra src.getCurChar() e l'EOL e li ritorna in out_result
											All'uscita di questa fn, src punta al primo char subito dopo EOL o a fine buffer
										*/

					bool			extractInteger (Source &src, i32 *out, const Char *validClosingChars = utf8::parser::CharArray_b_r_n_t, u32 numOfValidClosingChars=4);
										/*	Ritorna true se trova un valido intero, nel qual caso src punta al primo char subito dopo l'intero trovato
											Ritorna false se non ha trovato un intero, nel qual caso src rimane immodificato
											Un intero è valido se:
												- inizia con un '+' o un '-', seguiti da un numero, e tutti gli altri char sono numeri fino a fine buffer o fino a che non si incontra uno dei validClosingChars
												- inizia un numero, e tutti gli altri char sono numeri fino a fine buffer o fino a che non si incontra uno dei validClosingChars
										*/

					bool			extractIntArray (Source &src, i32 *out, u32 *maxIntIN_Out, const Char &arraySeparator=',');
										/*	Ritorna true se trova almeno 1 int valido.
											maxIntIN_Out indica il max num di int da inserire in *out e, in caso di successo, indica anche il num di int
											insertiti in out.
											La stringa deve contenere un int, oppure una serie di int separati da arraySeparator.										
											Ritorna false se non trova un array di int, nel qual caso src rimane invariato.
										*/

					bool			extractFloat (Source &src, f32 *out, const Char &sepDecimale='.', const Char *validClosingChars = utf8::parser::CharArray_b_r_n_t, u32 numOfValidClosingChars=4);
										/*	Ritorna true se trova un valido float, nel qual caso src punta al primo char subito dopo l'intero trovato
											Ritorna false se non ha trovato un float, nel qual caso src rimane immodificato
										*/

					bool			extractFloatArray (Source &src, f32 *out, u32 *maxFloatIN_Out, const Char &sepDecimale='.', const Char &arraySeparator=',');
										/*	Ritorna true se trova almeno 1 float valido.
											maxFloatIN_Out indica il max num di float da inserire in *out e, in caso di successo, indica anche il num di float
											insertiti in out.
											La stringa deve contenere un float, oppure una serie di float separati da arraySeparator.										
											Ritorna false se non trova un array di float, nel qual caso src rimane invariato.
										*/

					bool			extractIdentifier (Source &src, Source *out_result, const Char *otherValidChars=NULL, u32 numOfOtherValidChars=0);
										/*	Ritorna true se trova un valido identificatore, nel qual caso src punta al primo char subito dopo l'identificatore trovato e
											out_result punta all'identificatore.
											Ritorna false se non ha trovato un valido identifier, nel qual caso src rimane immodificato
											Un identificatore valido è una parola che:
												1- inizia con una lettera, oppure inizia con _ ed è seguito da una lettera
												2- tutti i caratteri oltre al primo sono lettere, numeri, _ oppure fanno parte di "*otherValidChars"
												3- termina non appena si trova un non numero, non lettera, non *otherValidChars
										*/

					bool			extractValue (Source &src, Source *out_result, const Char *validClosingChars= utf8::parser::CharArray_b_r_n_t, u32 numOfValidClosingChars=4);
										/*	Ritorna true se trova un valido value, nel qual caso src punta al primo char subito dopo il value trovato e
											out_result punta al value
											Ritorna false se non ha trovato un valido value, nel qual caso src rimane immodificato
											Un "value" valido è una o più parola che:
												1- se il primo carattere è un apice singolo (') o doppio (") allora "value" è tutti i caratteri compresi all'interno degli apici, ignorando i *validClosingChars
												2- altrimenti "value" deve iniziare con un "non spazio" e comprende tutti i caratteri fino a che non si trova un *validClosingChars
										*/							

					bool			extractCPPComment (Source &src, Source *out_result);
										/*	Ritorna true se trova un valido commento,nel qual caso src punta al primo char subito dopo il commento trovato e
											out_result punta al commento, comprensivo di / * * / o //
											Ritorna false se non ha trovato un valido value, nel qual caso src rimane immodificato
											Un commento è valido se:
												1- inizia con "//", allora è lungo fino alla fine della riga (\n\r o fine buffer)
												oppure
												2- inizia con /*, allora e finisce quando trova * /
										*/

					bool			find_CaseSens (Source &src, const char *whatTofind, u32 whatTofindLen=0);
					bool			find_NoCaseSens (Source &src, const char *whatTofind, u32 whatTofindLen=0);
										/*	Ritorna true se trova whatTofind, nel qual caso src punta al primo char di whatTofind
											Ritorna false se non ha trovato whatTofind, nel qual caso src rimane immodificato
											Se whatTofindLen==0, allora si suppone che whatTofind sia una stringa terminante per 0x00
										*/
		}; //namespace parser
	}; //namespace utf8
}; //namespace rhea
#endif //_rheaUTF8_h_