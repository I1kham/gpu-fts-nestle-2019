#ifndef _rheaString_h_
#define _rheaString_h_
#include <stdlib.h>
#include <string.h>
#include "OS/OS.h"
#include "rheaDataTypes.h"


namespace rhea
{
    //fwd declaration
    class Allocator;

	namespace string
	{
        //elimina tutti gli spazi a destra
        void	trimR (char *s);

        char*   alloc (rhea::Allocator *allocator, const char *src);

        // copia *src in *dst eventualmente troncando la copia a [sizeOfDest-1] caratteri
        void    copy_s (char *dst, size_t sizeOfDest, const char *src);

				void	append (char *dst, size_t sizeOfDest, const char *src);
				void	append(char *dst, size_t sizeOfDest, u32 num, u8 minNumOfDigit=0);
		inline	void	append(char *dst, size_t sizeOfDest, u16 num)								{ append(dst, sizeOfDest, (u32)num); }
		inline	void	append(char *dst, size_t sizeOfDest, u8 num)								{ append(dst, sizeOfDest, (u32)num); }
				void	append(char *dst, size_t sizeOfDest, i32 num, u8 minNumOfDigit = 0);
		inline	void	append(char *dst, size_t sizeOfDest, i16 num)								{ append(dst, sizeOfDest, (i32)num); }
		inline	void	append(char *dst, size_t sizeOfDest, i8 num)								{ append(dst, sizeOfDest, (i32)num); }
				void	append(char *dst, size_t sizeOfDest, char c);

		/*==============================================================================
		 * formattazione di vari tipi in stringa
		 *=============================================================================*/
		namespace format
		{
			void	F32 (f32 val, u32 numDecimal, char thousandSep, char decimalSep, char *out, u32 numCharInOut);
			void	U32 (u32 val, char thousandSep, char *out, u32 numCharInOut);
			void	U64 (u64 val, char thousandSep, char *out, u32 numCharInOut);
			void	Hex32 (u32 hex, char *out, u32 sizeofout);	//filla out con la rappresentazione esadecimale di hex (senza lo 0x davanti)
			void	Hex16 (u16 hex, char *out, u32 sizeofout);	//filla out con la rappresentazione esadecimale di hex (senza lo 0x davanti)
			void	Hex8 (u8 hex, char *out, u32 sizeofout);	//filla out con la rappresentazione esadecimale di hex (senza lo 0x davanti)
			void	timeMicroSecToHHMMSSMsUs (u64 microSec, char *out, u32 numCharInOut);

            void    currency (u16 price, u8 numDecimal, char decimalPointCharacter, char *out_s, u16 sizeOfOut);
        }

		/*==============================================================================
		 * conversioni da stringa a vari tipi
		 *=============================================================================*/
		namespace convert
		{
					bool	hexToInt (const char *s, u32 *out, u32 nBytes = u32MAX);
								/* s deve essere un hex valido, il che vuol dire:
										- inizia con un numero o con ABCDEF o abcdef
										- tutti i char successivi sono numeri o ABCDEF o abcdef fino a che non si trova 0x00 o nBytes==0
								*/

			inline	i32		toI32 (const char *s)														{ if (NULL==s) return 0; return (i32)atoi(s); }
			inline	u32		toU32 (const char *s)														{ if (NULL==s) return 0; return (u32)atoi(s); }
            inline	u64		toU64 (const char *s)														{ if (NULL==s) return 0; return (u64)atoll(s); }
					f32		toF32  (const char *s, u32 lenOfS=u32MAX);
					u32		hash (const char *str);
					u32		decodeURIinPlace(char *s);
							/* sostituisce le sequenze %xx con la relativa rappresentazione in byte
								Ritorna la nuova lunghezza di s
							*/
		};


		/*==============================================================================
		 * parsing delle stringhe
		 *=============================================================================*/
		namespace parser
		{
			struct Iter
			{
							Iter ()																	{ s=NULL; }
				void		setup (const char *src, u32 firstByte=0, u32 numByteToChek=u32MAX);
				void		toFirst()																{ iNow = iFirst; }
				void		toLast()																{ iNow = iLast; }
				void		next()																	{ if(iNow <= iLast) ++iNow; }
				void		prev()																	{ if(iNow >= iFirst) --iNow; }
				i32			cur() const																{ return iNow; }
				Iter&		operator=(const Iter &b)												{ copyFrom(b); return *this; }

				bool		cmp (const char *b, bool bCaseSensitive) const
							{ 
								const u32 n = getNumByteLeft();
								if (!n)
									return false;
								
								bool ret;
                                if (bCaseSensitive) ret = (strncmp (&s[iNow], b, n) == 0); else ret = (strncasecmp (&s[iNow], b, n) == 0);
								if (ret && b[n]==0) return true;
								return false;				
							}

				char		getCurChar() const														{ if (iNow<iFirst || iNow>iLast) return 0; return s[iNow]; }
                const char*	getCurStrPointer() const
                {
                    if (iNow<iFirst) return NULL;
                    if (iNow>iLast) return NULL;
                    return &s[iNow];
                }
				bool		isEOL() const															{ return getNumByteLeft()==0; }
				u32			getNumByteLeft() const													{ if (iNow<iFirst || iNow>iLast) return 0; return iLast - iNow +1; }
				void		copyCurStr (char *out, u32 sizeofOut) const									
							{
								if (isEOL()) out[0] = 0;
								else { assert (sizeofOut > getNumByteLeft()); memcpy (out, &s[iNow], getNumByteLeft()); out[getNumByteLeft()] = 0; }
							}
			private:
				void		copyFrom (const Iter &b)												{ s=b.s; iFirst=b.iFirst; iLast=b.iLast;iNow=b.iNow; }

			private:
				const char	*s;
				i32			iFirst;
				i32			iLast;
				i32			iNow;
			};


			inline	bool			isCharMaiuscolo (char c)											{ if (c>='A' && c<='Z') return true;  return false; }
			inline	bool			isCharMinuscolo (char c)											{ if (c>='a' && c<='z') return true; return false; }
			inline	bool			isANumber (char c)													{ if (c>='0' && c<='9') return true;  return false; }
			inline	bool			isALetter (char c)													{ if ((c>='a' && c<='z') || (c>='A' && c<='Z')) return true; return false; }
			inline	bool			isALetterOrANumber (char c)											{ if ((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')) return true; return false; }
			inline	bool			isAnHexDigit (char c)												{ if ((c>='a' && c<='f') || (c>='A' && c<='F') || (c>='0' && c<='9')) return true; return false; }
					bool			IsOneOfThis (char c, const char *validChars, u32 numOfValidChars);
										// ritorna true se c è uno dei *validChars


					void			skip (Iter &src, const char *toBeskippedChars, u32 numOfToBeskippedChars);
										/*	avanza e si ferma quando trova char != da quelli da skipppare o a fine buffer
											Se trova un char != dai toBeskippedChars, src punta al primo char trovato
										*/

					void			skipEOL (Iter &src);
										/*	se src punta a \r, allora skippa \r e l'evenutale \n che segue.
											Se src punta a \n, lo skippa
										*/


					bool			advanceUntil (Iter &src, const char *validTerminators, u32 numOfValidTerminators);
										/*	controlla il char corrente e avanza fino a quando non trova uno dei validTerminators o a fine buffer
											Ritorna true se trova uno dei validTerminators, nel qual caso src punta al primo validTerminators trovato.
											Ritorna false se non trova un validTerminators, nel qual caso src punta a fine buffer
										*/

					bool			backUntil (Iter &src, const char *validTerminators, u32 numOfValidTerminators);
										/*	controlla il char corrente e indietreggia fino a quando non trova uno dei validTerminators o a inizio buffer
											Ritorna true se trova uno dei validTerminators, nel qual caso src punta al primo validTerminators trovato.
											Ritorna false se non trova un validTerminators, nel qual caso src punta a inizio buffer
										*/

			inline	void			toNextValidChar (Iter &src)										{ parser::skip (src, " \r\n\t", 4); }
										/*	usa la skip() per skippare tutti o "\r\n\t\b" e ritorna l'indice del primo char buono 
										*/
		
					void			extractLine (Iter &src, Iter *out_result);
										/*	Prende tutti i caratteri compresi tra src.getCurChar() e l'EOL e li ritorna in out_result
											All'uscita di questa fn, src punta al primo char subito dopo EOL o a fine buffer
										*/
								
					bool			extractInteger (Iter &src, i32 *out, const char *validClosingChars = " \r\n\t", u32 numOfValidClosingChars=4);
										/*	Ritorna true se trova un valido intero, nel qual caso src punta al primo char subito dopo l'intero trovato
											Ritorna false se non ha trovato un intero, nel qual caso src rimane immodificato
											Un intero è valido se:
												- inizia con un '+' o un '-', seguiti da un numero, e tutti gli altri char sono numeri fino a fine buffer o fino a che non si incontra uno dei validClosingChars
												- inizia con un numero, e tutti gli altri char sono numeri fino a fine buffer o fino a che non si incontra uno dei validClosingChars
										*/

					bool			extractIntArray (Iter &src, i32 *out, u32 *maxIntIN_Out, char arraySeparator=',');
										/*	Ritorna true se trova almeno 1 int valido.
											maxIntIN_Out indica il max num di int da inserire in *out e, in caso di successo, indica anche il num di int
											insertiti in out.
											La stringa deve contenere un int, oppure una serie di int separati da arraySeparator.										
											Ritorna false se non trova un array di int, nel qual caso src rimane invariato.
										*/

					bool			extractFloat (Iter &src, f32 *out, char sepDecimale='.', const char *validClosingChars = " \r\n\t", u32 numOfValidClosingChars=4);
										/*	Ritorna true se trova un valido float, nel qual caso src punta al primo char subito dopo l'intero trovato
											Ritorna false se non ha trovato un float, nel qual caso src rimane immodificato
										*/

					bool			extractFloatArray (Iter &src, f32 *out, u32 *maxFloatIN_Out, char sepDecimale='.', char arraySeparator=',');
										/*	Ritorna true se trova almeno 1 float valido.
											maxFloatIN_Out indica il max num di float da inserire in *out e, in caso di successo, indica anche il num di float
											insertiti in out.
											La stringa deve contenere un float, oppure una serie di float separati da arraySeparator.										
											Ritorna false se non trova un array di float, nel qual caso src rimane invariato.
										*/

					bool			extractIdentifier (Iter &src, Iter *out_result, const char *otherValidChars=NULL, u32 numOfOtherValidChars=0);
										/*	Ritorna true se trova un valido identificatore, nel qual caso src punta al primo char subito dopo l'identificatore trovato e
											out_result punta all'identificatore.
											Ritorna false se non ha trovato un valido identifier, nel qual caso src rimane immodificato
											Un identificatore valido è una parola che:
												1- inizia con una lettera, oppure inizia con _ ed è seguito da una lettera
												2- tutti i caratteri oltre al primo sono lettere, numeri, _, oppure fanno parte di "*otherValidChars"
												3- termina non appena si trova un non numero, non lettera, non *otherValidChars
										*/

					bool			extractValue (Iter &src, Iter *out_result, const char *validClosingChars, u32 numOfValidClosingChars);
										/*	Ritorna true se trova un valido value, nel qual caso src punta al primo char subito dopo il value trovato e
											out_result punta al value
											Ritorna false se non ha trovato un valido value, nel qual caso src rimane immodificato
											Un "value" valido è una o più parola che:
												1- se il primo carattere è un apice singolo (') o doppio (") allora "value" è tutti i caratteri compresi all'interno degli apici, ignorando i *validClosingChars
												2- altrimenti "value" deve iniziare con un "non spazio" e comprende tutti i caratteri fino a che non si trova un *validClosingChars oppure fino a fine buffer
										*/							

					bool			extractCPPComment (Iter &src, Iter *out_result);
										/*	Ritorna true se trova un valido commento,nel qual caso src punta al primo char subito dopo il commento trovato e
											out_result punta al commento, comprensivo di / * * / o //
											Ritorna false se non ha trovato un valido commento, nel qual caso src rimane immodificato
											Un commento è valido se:
												1- inizia con "//", allora è lungo fino alla fine della riga (\n\r o fine buffer)
												oppure
                                                2- inizia con / *, allora e finisce quando trova * /
										*/

					
					bool			extractFileExt	(const Iter &src, Iter *out_result);
										/*	Si porta a fine stringa e poi va indietro di 1 char alla volta fino a che non trova un "." 
											src rimane immutato
										*/
					bool			extractFileNameWithExt	(const Iter &src, Iter *out_result);
					bool			extractFileNameWithoutExt (const Iter &src, Iter *out_result);
										/*	Si portano a fine stringa e poi vanno indietro di 1 char alla volta fino a che non trovano un "/" o inizio buffer
											src rimane immutato
										*/						

					bool			extractFilePathWithSlash	(const Iter &src, Iter *out_result);
					bool			extractFilePathWithOutSlash	(const Iter &src, Iter *out_result);
										/*	Si portano a fine stringa e poi vanno indietro di 1 char alla volta fino a che non trovano un "/" o inizio buffer
											Il path sono tutti i char da inizio stringa fino all'ultimo / trovato
											src rimane immutato
										*/						

		}; //namespace parser
    }; //namespace string
}; //namespace rhea
#endif //_rheaString_h_
