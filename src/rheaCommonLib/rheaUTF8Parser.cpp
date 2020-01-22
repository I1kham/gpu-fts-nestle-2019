#include "rheaUTF8.h"
#include "OS/OS.h"
#include "rheaString.h"


using namespace rhea;
using namespace rhea::utf8;

namespace rhea
{
	namespace utf8
	{
		namespace parser
		{
			Char	CharArray_b_r_n_t[4] = { utf8::Char(' '), utf8::Char('\r'), utf8::Char('\n'), utf8::Char('\t') };		
			Char	CharArray_sepParole[6] = { utf8::Char(' '), utf8::Char(','), utf8::Char('.'), utf8::Char(';'), utf8::Char(':'), utf8::Char('\t') };
		};
	};
};


//*******************************************
bool is_b_r_n_t (const utf8::Char &c)
{
	return parser::IsOneOfThis ( c, utf8::parser::CharArray_b_r_n_t, 4);
}


//*******************************************
void parser::Source::setup (const u8 *src, u32 firstByte, u32 maxByteToChek)
{
	s = src; 
	iNow = firstByte; 
	if (numByteToCheck == u32MAX)
		numByteToCheck = (u32)strlen((const char*)src);
	else
		numByteToCheck = maxByteToChek; 
	curChar.setFrom (&s[iNow], numByteToCheck);
}

//*******************************************
bool parser::Source::next()
{ 
	if (s[iNow] == 0x00 || numByteToCheck==0 || curChar.isNULL())
		return false; 
	u8 n = curChar.len();
	iNow += n;
	numByteToCheck -= n; 
	if (s[iNow] == 0x00 || numByteToCheck==0)
	{
		curChar.setNULL();
		return false; 
	}
	curChar.setFrom (&s[iNow], numByteToCheck);
	return true; 
}

//*******************************************
bool parser::Source::advance (u32 nChar)
{
	while (nChar--)
		if (!next())
			return false;
	return true;
}

//*******************************************
bool parser::Source::prev()
{ 
	if (iNow == 0 || numByteToCheck==0)
		return false; 

	u8 n;
	if (!utf8::extractOneUTF8CharRev (s, iNow, &n))
		return false;
	if (n > iNow)
		return false;
	iNow -= n;
	numByteToCheck+=n;
	curChar.setFrom (&s[iNow], n);
	return true;
}

//*******************************************
bool parser::Source::toFirst()
{
	iNow = 0;
	if (numByteToCheck==0)
		curChar.setNULL();
	else
		curChar.setFrom (s, numByteToCheck);
	return true;
}

//*******************************************
bool parser::Source::toLast()
{
	assert (numByteToCheck < 0xFFFFF000);
	if (numByteToCheck==0)
	{
		toFirst();
		return false;
	}

	u8 n = utf8::detectCharlenInByteByFirstByte (s[numByteToCheck-1]);
	if (0 == n || n > numByteToCheck)
	{
		toFirst();
		return false;
	}

	iNow = numByteToCheck - n;
	curChar.setFrom (&s[iNow], n);
	return true;
}

//*****************************************
bool parser::IsOneOfThis (const Char &c, const Char *validChars, u32 numOfValidChars)
{
	if (numOfValidChars == 0 || NULL == validChars)
		return false;
	for (u32 i2=0; i2<numOfValidChars; i2++)
	{
		if (c == validChars[i2])
			return true;
	}
	return false;
}

//*****************************************
void parser::skip (parser::Source &src, const Char *toBeskippedChars, u32 numOfToBeskippedChars)
{
	while (parser::IsOneOfThis (src.getCurChar(), toBeskippedChars, numOfToBeskippedChars))
		if (!src.next())
			return;
}

//*****************************************
void parser::skipBack (parser::Source &src, const Char *toBeskippedChars, u32 numOfToBeskippedChars)
{
	while (parser::IsOneOfThis (src.getCurChar(), toBeskippedChars, numOfToBeskippedChars))
		if (!src.prev())
			return;
}

//*****************************************
void parser::gotoChar (parser::Source &src, u32 n)
{
	src.toFirst();
	while (!src.getCurChar().isNULL() && n--)
		if (!src.next())
			return;
}


//*****************************************
bool parser::advanceUntil (parser::Source &src, const Char *validTerminators, u32 numOfValidTerminators)
{
	while (!parser::IsOneOfThis (src.getCurChar(), validTerminators, numOfValidTerminators))
	{
		if (!src.next())
			return false;
	}
	return true;
}

//*****************************************
void parser::advanceToEOL (Source &src, bool bskipEOL)
{
	while (!src.getCurChar().isNULL())
	{
		if (src.getCurChar()=='\r' || src.getCurChar()=='\n')
		{
			if (bskipEOL)
				parser::skipEOL(src);
			return;
		}
		src.next();
	}
}

//*****************************************
bool parser::backUntil (Source &src, const Char *validTerminators, u32 numOfValidTerminators)
{
	while (!src.getCurChar().isNULL())
	{
		if (parser::IsOneOfThis (src.getCurChar(), validTerminators, numOfValidTerminators))
			return true;
		if (!src.prev())
			return false;
	}
	return true;
}

//*****************************************
void parser::skipEOL (parser::Source &src)
{
	if (src.getCurChar() == '\n')
	{
		src.next();
		return;
	}
	if (src.getCurChar() == '\r')
	{
		src.next();
		if (src.getCurChar() == '\n')
			src.next();
		return;
	}
}

//*****************************************
void parser::extractLine (parser::Source &srcIN, parser::Source *out_result)
{
	assert (out_result);
	Source src = srcIN;
	
	*out_result = srcIN;
	out_result->numByteToCheck = 0;
	while (!src.getCurChar().isNULL())
	{
		if (src.getCurChar() == '\n' || src.getCurChar() == '\r')
			break;
		src.next();
	}

	out_result->numByteToCheck = src.iNow - srcIN.iNow;
	if (src.getCurChar() == '\n' || src.getCurChar() == '\r')
		skipEOL (src);
	srcIN = src;
}

//*****************************************
bool parser::extractInteger (parser::Source &srcIN, i32 *out, const Char *validClosingChars, u32 numOfValidClosingChars)
{
	parser::Source src = srcIN;
	
	parser::toNextValidChar( src );
	const u32 firstGoodChar = src.iNow;

	//il primo char potrebbe essere +/-
	if (src.getCurChar() == '+' || src.getCurChar() == '-')
		src.next();

	//qui deve esserci un numero
	if (!parser::isANumber(src.getCurChar()))
		return false;
	src.next();

	//da ora in poi, devono essere tutti numeri
	while (parser::isANumber(src.getCurChar()))
		if (!src.next())
			break;

	//ho trovato fine buffer o un char che non è un numero.
	//se è un valido separatore, ok, altrimenti è un errore
	while (1)
	{
		if ( src.isEOL() )
			break;
		if ( is_b_r_n_t (src.getCurChar()) )
			break;
		if ( parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars) )
			break;
		return false;
	}

	//se arrivo qui vuol dire che la stringa conteneva un num valido e curChar() punta al separatore o a fine buffer
	u32 numChar = src.iNow - firstGoodChar;

	//converto in int
	const u32 MAXTEMP = 63;
	char temp[MAXTEMP+1];
	if (numChar > MAXTEMP)
	{
		DBGBREAK;
		numChar = MAXTEMP;
	}
	memcpy (temp, &src.s[firstGoodChar], numChar);
	temp[numChar] = 0;
	*out = string::convert::toI32 (temp);
	srcIN = src;
	return true;
}

//*****************************************
bool parser::extractIntArray (parser::Source &srcIN, i32 *out, u32 *maxIntIN_Out, const Char &arraySeparator)
{
	assert (*maxIntIN_Out > 0);
	assert (arraySeparator!=' ');
	
	u32 nOUT = 0;
	Source src = srcIN;

	//parso
	Char intEndingChar[3] = { Char(' '), Char('\t'), arraySeparator };
	Char array_b_t[2] = { Char(' '), Char('\t') };

	while (nOUT < *maxIntIN_Out)
	{
		parser::skip (src, array_b_t, 2);
		if (!utf8::parser::extractInteger (src, &out[nOUT], intEndingChar, 3))
			break;
		++nOUT;

		//cerco il prossimo array separator
		if (src.getCurChar() != arraySeparator)
			parser::skip (src, array_b_t, 2);

		if (src.getCurChar().isNULL())
			break;
		if (src.getCurChar() != arraySeparator)
			return false;
		src.next();

		if (nOUT >= *maxIntIN_Out)
			break;
	}

	*maxIntIN_Out = nOUT;
	if (nOUT == 0)
		return false;
	srcIN = src;
	return true;
}

//*****************************************
bool parser::extractFloat (parser::Source &srcIN, f32 *out, const Char &sepDecimale, const Char *validClosingChars, u32 numOfValidClosingChars)
{
	assert (sepDecimale != ' ');

	Source	src = srcIN;
	bool	sepDecimaleFound = false;
	bool	eFound = false;

	//il primo char potrebbe essere il sepDecimale
	if (src.getCurChar() == sepDecimale)
	{
		sepDecimaleFound = true;
		src.next();
	}
	// oppure potrebbe essere un +/-
	else if (src.getCurChar() == '+' || src.getCurChar() == '-')
		src.next();

	//qui deve esserci un numero
	if (!parser::isANumber(src.getCurChar()))
		return false;
	src.next();


	//da ora in poi, devono essere tutti numeri, sono ammessi anche un solo sepDecimale, e un "e" o "E" se siamo nella parte intera
	Char c;
	while (!(c=src.getCurChar()).isNULL())
	{
		if (parser::isANumber(c))
		{
			src.next();
			continue;
		}
		if (c == '.')
		{
			if (sepDecimaleFound)
				return false;
			if (eFound)
				return false;
			sepDecimaleFound = true;
			src.next();
			continue;
		}
		if (parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars))
			break;

		if (c == 'e' || c=='E')
		{
			if (eFound)
				return false;
			eFound = true;

			//subito dopo potrebbe esserci un +/-
			src.next();
			if (src.getCurChar() == '+' || src.getCurChar() == '-')
				src.next();

			//qui deve esserci un numero
			if (!parser::isANumber(src.getCurChar()))
				return false;
			src.next();
			continue;
		}

		break;
	}

	//ho trovato fine buffer o un char che invalido.
	//se il char è un valido separatore, ok, altrimenti è un errore
	if (!src.getCurChar().isNULL() && !parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars))
		return false;

	//se arrivo qui vuol dire che la stringa conteneva un num valido e curChar() punta al separatore o a fine buffer
	u32 numChar = src.iNow - srcIN.iNow;

	//converto in float
	const u32 MAXTEMP = 63;
	char temp[MAXTEMP+1];
	if (numChar > MAXTEMP)
	{
		DBGBREAK;
		numChar = MAXTEMP;
	}
	memcpy (temp, srcIN.getCurStrPointer(), numChar);
	temp[numChar] = 0;
	*out = string::convert::toF32 (temp);
	srcIN = src;
	return true;
}

//*****************************************
bool parser::extractFloatArray (parser::Source &srcIN, f32 *out, u32 *maxFloatIN_Out, const Char &sepDecimale, const Char &arraySeparator)
{
	assert (*maxFloatIN_Out > 0);
	assert (sepDecimale!=' ' && arraySeparator!=sepDecimale);
	
	u32 nOUT = 0;
	Source src = srcIN;

	//parso
	Char floatEndingChar[5] = { Char(' '), Char('\t'), arraySeparator, Char('\r'), Char('\n')};
	Char eol[3] = { Char('\r'), Char('\n') };
	Char array_b_t[2] = { Char(' '), Char('\t') };
	while (nOUT < *maxFloatIN_Out)
	{
		parser::skip (src, array_b_t, 2);
		if (!utf8::parser::extractFloat (src, &out[nOUT], sepDecimale, floatEndingChar, 5))
			break;
		++nOUT;

		//cerco il prossimo array separator
		if (src.getCurChar() != arraySeparator)
			parser::skip (src, array_b_t, 2);

		if (src.getCurChar().isNULL() || utf8::parser::IsOneOfThis (src.getCurChar(), eol, 2) )
			break;
		if (src.getCurChar() != arraySeparator)
			return false;
		src.next();

		if (nOUT >= *maxFloatIN_Out)
			break;
	}

	*maxFloatIN_Out = nOUT;
	if (nOUT == 0)
		return false;
	srcIN = src;
	return true;

}

//*****************************************
bool parser::extractIdentifier (parser::Source &srcIN, parser::Source *result, const Char *otherValidChars, u32 numOfOtherValidChars)
{
	Source	src = srcIN;

	//il primo char potrebbe essere "_"
	if (src.getCurChar() == '_')
		src.next();

	//qui deve esserci una lettera
	if (!parser::isALetter(src.getCurChar()))
		return false;
	src.next();

	while (!src.getCurChar().isNULL())
	{
		if (src.getCurChar()=='_' || parser::isALetterOrANumber(src.getCurChar()) || parser::IsOneOfThis (src.getCurChar(), otherValidChars, numOfOtherValidChars))
		{
			src.next();
			continue;
		}
		break;
	}

	//ho trovato fine buffer o un char invalido, quindi va tutto bene
	result->setup (&(srcIN.s[srcIN.iNow]), 0, src.iNow - srcIN.iNow );
	srcIN = src;
	return true;
}


//*****************************************
bool parser::extractValue (parser::Source &srcIN, parser::Source *result, const Char *validClosingChars, u32 numOfValidClosingChars)
{
	Source src = srcIN;
	if (src.getCurChar() =='"' || src.getCurChar()=='\'')
	{
		//il value è racchiuso tra apici. Prendo tutto fino a che non trovo un'altro apice uguale all'apice di apertura
		Char opening = src.getCurChar();
		src.next();

		if (!advanceUntil (src, &opening, 1))
			//non ho trovato l'opening finale
			return false;
		src.next();

		result->setup (&(srcIN.s[srcIN.iNow + 1]), 0, src.iNow - srcIN.iNow -2);
		srcIN = src;
		return true;
	}
	else
	{
		// l'identificatore non è racchiuso tra apici per cui prendo tutto fino a che non trovo un validClosingChars o fine buffer

		//il primo char però non deve essere uno spazio
		if (parser::IsOneOfThis (src.getCurChar(), CharArray_b_r_n_t, 4))
			return false;

		if (!advanceUntil (src, validClosingChars, numOfValidClosingChars))
		{
			//se non ho trovato un valido sep, ma sono a fine buffer, va bene lo stesso
			if (!src.getCurChar().isNULL())
				return false;
		}

		result->setup (&(srcIN.s[srcIN.iNow]), 0, src.iNow - srcIN.iNow );
		srcIN = src;
		return true;
	}
}

//*****************************************
bool parser::extractCPPComment (parser::Source &srcIN, parser::Source *result)
{
	Source src = srcIN;
	if (src.getCurChar() != '/')
		return false;
	src.next();

	//se il secondo char è un'altro /, leggo fino a fine riga
	if (src.getCurChar() == '/')
	{
		parser::extractLine (srcIN, result);
		return true;
	}
	
	//se il secondo char è * leggo fino a che non trovo * /
	if (src.getCurChar() == '*')
	{
		Char star('*');
		while (1)
		{
			if (!advanceUntil (src, &star, 1))
				return false;
			src.next();
			if (src.getCurChar() == '/')
			{
				src.next();
				result->setup (&(srcIN.s[srcIN.iNow]), 0, src.iNow - srcIN.iNow );
				srcIN = src;
				return true;
			}
		}
	}

	return false;
}

//*****************************************
bool parser::find_CaseSens (parser::Source &srcIN, const u8 *whatTofind, u32 whatTofindLen)
{
	assert (NULL != whatTofind);
	parser::Source src = srcIN;
	if (0 == whatTofindLen)
		whatTofindLen = (u32)strlen((const char*)whatTofind);

	
	parser::Source what;
	what.setup (whatTofind, 0, whatTofindLen);	

	while (!src.getCurChar().isNULL())
	{
		if (src.getCurChar() != what.getCurChar())
		{
			src.next();
			continue;
		}


		parser::Source temp = src;
		while (src.getCurChar() == what.getCurChar())
		{
			if (!what.next())
			{
				srcIN = temp;
				return true;
			}

			if (!src.next())
				return false;
		}

		what.setup (whatTofind, 0, whatTofindLen);
		src = temp;
		src.next();
	}

	return false;
}

//*****************************************
bool parser::find_NoCaseSens (parser::Source &srcIN, const u8 *whatTofind, u32 whatTofindLen)
{
	assert (NULL != whatTofind);
	parser::Source src = srcIN;
	if (0 == whatTofindLen)
		whatTofindLen = (u32)strlen((const char*)whatTofind);

	
	parser::Source what;
	what.setup (whatTofind, 0, whatTofindLen);
	what.toFirst();

	while (!src.getCurChar().isNULL())
	{
		if (!src.getCurChar().isEqualNoCaseSensitive (what.getCurChar()))
		{
			src.next();
			continue;
		}


		parser::Source temp = src;
		while (src.getCurChar().isEqualNoCaseSensitive (what.getCurChar()))
		{
			if (!what.next())
			{
				srcIN = temp;
				return true;
			}

			if (!src.next())
				return false;
		}

		what.setup (whatTofind, 0, whatTofindLen);
		src = temp;
		src.next();
	}

	return false;
}




//*******************************
bool parser::advancetoNextWord (parser::Source &src)
{
	if (!parser::advanceUntil (src, CharArray_sepParole, 6))
		return false;
	parser::skip (src, CharArray_sepParole, 6);

	if (src.getCurChar().isNULL())
		return false;
	return true;
}

//*******************************
bool parser::backToPreviousWord (parser::Source &src)
{
	parser::skipBack (src, CharArray_sepParole, 6);
	if (!parser::backUntil (src, CharArray_sepParole, 6))
		return false;
	parser::skip (src, CharArray_sepParole, 6);
	return true;
}