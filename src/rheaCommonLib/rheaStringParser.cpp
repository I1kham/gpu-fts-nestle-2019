#include "rheaString.h"

using namespace rhea;
using namespace rhea::string;

//*****************************************
bool parser::IsOneOfThis (char c, const char *validChars, u32 numOfValidChars)
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
void parser::skip (parser::Iter &src, const char *toBeskippedChars, u32 numOfToBeskippedChars)
{
	assert (numOfToBeskippedChars);
	while (src.getCurChar())
	{
		if (!parser::IsOneOfThis (src.getCurChar(), toBeskippedChars, numOfToBeskippedChars))
			break;
		src.next();
	}
}

//*****************************************
void parser::skipEOL (parser::Iter &src)
{
	if (src.getCurChar() == '\n')
	{
		src.next();
		if (src.getCurChar() == '\r')
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
bool parser::advanceUntil (parser::Iter &src, const char *validTerminators, u32 numOfValidTerminators)
{
	assert (numOfValidTerminators);
	while (src.getCurChar())
	{
		if (parser::IsOneOfThis (src.getCurChar(), validTerminators, numOfValidTerminators))
			return true;
		src.next();

	}
	return false;
}

//*****************************************
bool parser::backUntil (parser::Iter &src, const char *validTerminators, u32 numOfValidTerminators)
{
	assert (numOfValidTerminators);
	while (src.getCurChar())
	{
		if (parser::IsOneOfThis (src.getCurChar(), validTerminators, numOfValidTerminators))
			return true;
		src.prev();
	}
	return false;
}



//*****************************************
void parser::extractLine (parser::Iter &srcIN, parser::Iter *out_result)
{
	const char *s1 = srcIN.getCurStrPointer();
	if (!s1)
	{
		*out_result = srcIN;
		return;
	}

	parser::Iter src = srcIN;
	while (src.getCurChar())
	{
		if (src.getCurChar() == '\n' || src.getCurChar() == '\r')
		{
			const char *s2 = src.getCurStrPointer();
			out_result->setup (s1, 0, (u32)(s2-s1));
			parser::skipEOL (src);
			srcIN = src;
			return;
		}
		src.next();
	}

	*out_result = srcIN;
	srcIN = src;
}


//*****************************************
bool parser::extractInteger (parser::Iter &srcIN, i32 *out, const char *validClosingChars, u32 numOfValidClosingChars)
{
	parser::Iter src = srcIN;
	
	const char *s1 = src.getCurStrPointer();
	if (NULL == s1)
		return false;

	//il primo char potrebbe essere +/-
	if (s1[0] == '+' || s1[0] == '-')
	{
		src.next();

		//eventuali spazi
		parser::skip (src, " ", 1);
	}

	//qui deve esserci un numero
	if (!parser::isANumber(src.getCurChar()))
		return false;
	src.next();

	//da ora in poi, devono essere tutti numeri
	while (parser::isANumber(src.getCurChar()))
		src.next();

	//ho trovato fine buffer o un char che non è un numero.
	//se è un valido separatore, ok, altrimenti è un errore
	if (src.getCurChar()!=0x00 && !parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars))
		return false;



	//se arrivo qui vuol dire che la stringa conteneva un num valido e curChar() punta al separatore o a fine buffer
	srcIN = src;

	src.prev();
	const char *s2 = src.getCurStrPointer();
	assert (s2);
	u32 numChar = (u32)(s2-s1) +1;

	//converto in int
	const u32 MAXTEMP = 63;
	char temp[MAXTEMP+1];
	if (numChar > MAXTEMP)
	{
		DBGBREAK;
		numChar = MAXTEMP;
	}
	memcpy (temp, s1, numChar);
	temp[numChar] = 0;
	*out = string::convert::toI32 (temp);
	return true;
}

//*****************************************
bool parser::extractIntArray (parser::Iter &srcIN, i32 *out, u32 *maxIntIN_Out, char arraySeparator)
{
	assert (*maxIntIN_Out > 0);
	assert (arraySeparator!=' ');
	
	u32 nOUT = 0;
	parser::Iter src = srcIN;

	//parso
	char intEndingChar[4] = {' ', '\t', arraySeparator, 0};
	u32 maxN = *maxIntIN_Out;
	while (nOUT < maxN)
	{
		parser::skip (src, " \t", 2);
		if (!string::parser::extractInteger (src, &out[nOUT], intEndingChar, 3))
			break;
		++nOUT;

		//cerco il prossimo array separator
		if (src.getCurChar() != arraySeparator)
			parser::skip (src, " \t", 2);

		if (src.getCurChar() == 0x00)
			break;
		if (src.getCurChar() != arraySeparator)
			return false;
		src.next();

		if (nOUT >= maxN)
			break;
	}

	*maxIntIN_Out = nOUT;
	if (nOUT == 0)
		return false;
	srcIN = src;
	return true;
}

//*****************************************
bool parser::extractFloat (parser::Iter &srcIN, f32 *out, char sepDecimale, const char *validClosingChars, u32 numOfValidClosingChars)
{
	assert (sepDecimale != ' ');


	parser::Iter	src = srcIN;

	const char *s1 = src.getCurStrPointer();
	if (NULL == s1)
		return false;

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
	{
		src.next();
		//eventuali spazi
		parser::skip (src, " ", 1);
	}

	//qui deve esserci un numero
	if (!parser::isANumber(src.getCurChar()))
		return false;
	src.next();


	//da ora in poi, devono essere tutti numeri, sono ammessi anche un solo sepDecimale, e un "e" o "E" se siamo nella parte intera
	char c;
	while ((c=src.getCurChar()))
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

		if (parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars))
			break;
	}

	//ho trovato fine buffer o un char che invalido.
	//se il char è un valido separatore, ok, altrimenti è un errore
	if (src.getCurChar()!=0x00 && !parser::IsOneOfThis (src.getCurChar(), validClosingChars, numOfValidClosingChars))
		return false;


	//se arrivo qui vuol dire che la stringa conteneva un num valido e curChar() punta al separatore o a fine buffer
	srcIN = src;
	src.prev();
	const char *s2 = src.getCurStrPointer();
	assert (s2);
	u32 numChar = (u32)(s2-s1) +1;

	//converto in float
	const u32 MAXTEMP = 63;
	char temp[MAXTEMP+1];
	if (numChar > MAXTEMP)
	{
		DBGBREAK;
		numChar = MAXTEMP;
	}
	memcpy (temp, s1, numChar);
	temp[numChar] = 0;
	*out = string::convert::toF32 (temp);
	return true;
}

//*****************************************
bool parser::extractFloatArray (parser::Iter &srcIN, f32 *out, u32 *maxFloatIN_Out, char sepDecimale, char arraySeparator)
{
    assert (maxFloatIN_Out != NULL);
	assert (arraySeparator!=' ' && sepDecimale!=' ' && arraySeparator!=sepDecimale);
	
	u32 nOUT = 0;
	Iter src = srcIN;

	//parso
	char floatEndingChar[4] = {' ', '\t', arraySeparator, 0};
	u32 n = *maxFloatIN_Out;
	while (nOUT < n)
	{
		parser::skip (src, " \t", 2);
		if (!string::parser::extractFloat (src, &out[nOUT], sepDecimale, floatEndingChar, 3))
			break;
		++nOUT;

		//cerco il prossimo array separator
		if (src.getCurChar() != arraySeparator)
			parser::skip (src, " \t", 2);

		if (src.getCurChar() == 0x00)
			break;
		if (src.getCurChar() != arraySeparator)
			return false;
		src.next();

		if (nOUT >= n)
			break;
	}

	*maxFloatIN_Out = nOUT;
	if (nOUT == 0)
		return false;
	srcIN = src;
	return true;

}

//*****************************************
bool parser::extractIdentifier (parser::Iter &srcIN, parser::Iter *result, const char *otherValidChars, u32 numOfOtherValidChars)
{
	parser::Iter src = srcIN;

	const char *s1 = src.getCurStrPointer();
	if (NULL == s1)
		return false;

	//il primo char potrebbe essere "_"
	if (src.getCurChar() == '_')
		src.next();

	//qui deve esserci una lettera
	if (!parser::isALetter(src.getCurChar()))
		return false;
	src.next();

	char c;
	while ((c = src.getCurChar()))
	{
		if (c=='_' || parser::isALetterOrANumber(c) || parser::IsOneOfThis (c, otherValidChars, numOfOtherValidChars))
		{
			src.next();
			continue;
		}
		break;
	}

	//ho trovato fine buffer o un char invalido, quindi va tutto bene
	srcIN = src;

	src.prev();
	const char *s2 = src.getCurStrPointer();
	assert (s2);
	const u32 numChar = (u32)(s2-s1) +1;
	result->setup (s1, 0, numChar);
	return true;
}


//*****************************************
bool parser::extractValue (parser::Iter &srcIN, parser::Iter *result, const char *validClosingChars, u32 numOfValidClosingChars)
{
	parser::Iter src = srcIN;
	const char *s1 = src.getCurStrPointer();
	if (NULL == s1)
		return false;

	if (src.getCurChar() == '"' || src.getCurChar()== '\'' )
	{
		//il value è racchiuso tra apici. Prendo tutto fino a che non trovo un'altro apice uguale all'apice di apertura
		char opening[2] = {src.getCurChar(), 0};
		src.next();
		++s1;
		if (!advanceUntil (src, opening, 1))
			//non ho trovato l'opening finale
			return false;

		srcIN = src;

		src.prev();
		const char *s2 = src.getCurStrPointer();
		assert (s2);
		const u32 numChar = (u32)(s2-s1) +1;
		result->setup (s1, 0, numChar);
		return true;
	}
	else
	{
		// l'identificatore non è racchiuso tra apici per cui prendo tutto fino a che non trovo un validClosingChars o fine buffer

		//il primo char però non deve essere uno spazio
		if (parser::IsOneOfThis (src.getCurChar(), " \r\n\t", 4))
			return false;

		if (!advanceUntil (src, validClosingChars, numOfValidClosingChars))
		{
			//se non ho trovato un valido sep, ma sono a fine buffer, va bene lo stesso
			if (src.getCurChar() != 0x00)
				return false;
		}
		srcIN = src;
		srcIN.next(); //passo al prox carattere, quello subito dopo al terminatore trovato

		src.prev();
		const char *s2 = src.getCurStrPointer();
		if (s2 >= s1)
		{
			const u32 numChar = (u32)(s2-s1) +1;
			result->setup (s1, 0, numChar);
			return true;
		}
		return false;
	}
}

//*****************************************
bool parser::extractCPPComment (parser::Iter &srcIN, parser::Iter *result)
{
	parser::Iter src = srcIN;

	const char *s1 = src.getCurStrPointer();
	if (NULL == s1)
		return false;

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
		while (1)
		{
			if (!advanceUntil (src, "*", 1))
				return false;
			src.next();
			if (src.getCurChar() == '/')
			{
				const char *s2 = src.getCurStrPointer();
				assert (s2);
				const u32 numChar = (u32)(s2-s1) +1;
				result->setup (s1, 0, numChar);

				src.next();
				srcIN = src;
				return true;
			}
		}
	}

	return false;
}

//*****************************************
bool parser::extractFileExt	(const parser::Iter &srcIN, parser::Iter *out_result)
{
	parser::Iter src = srcIN;

	src.toLast();
	const char *s2 = src.getCurStrPointer();
	if (NULL == s2)
		return false;

	char c;
	while ((c = src.getCurChar()))
	{
		if (c == '.')
		{
			src.next();
			const char *s1 = src.getCurStrPointer();
			assert (s1);
			if (s2 >= s1)
				out_result->setup (s1, 0, (u32)(s2-s1) +1);
			else
				out_result->setup (s1, 0, 0);
			return true;
		}
		src.prev();
	}
	return false;
}

//*****************************************
bool parser::extractFileNameWithExt	(const parser::Iter &srcIN, parser::Iter *out_result)
{
	parser::Iter src = srcIN;

	src.toLast();
	const char *s2 = src.getCurStrPointer();
	if (NULL == s2)
		return false;

	char c;
	while ((c = src.getCurChar()))
	{
		if (c == '/')
		{
			src.next();
			const char *s1 = src.getCurStrPointer();
			assert (s1);
			if (s2 >= s1)
				out_result->setup (s1, 0, (u32)(s2-s1) + 1);
			else
				out_result->setup (s1, 0, 0);
			return true;
		}
		src.prev();
	}

	*out_result = srcIN;
	return true;
}

//*****************************************
bool parser::extractFileNameWithoutExt	(const parser::Iter &srcIN, parser::Iter *out_result)
{
	parser::Iter filenameWithExt;
	if (!parser::extractFileNameWithExt(srcIN, &filenameWithExt))
		return false;

	parser::Iter src = filenameWithExt;
	src.toLast();
	if (NULL == src.getCurStrPointer())
		return false;

	char c;
	while ((c = src.getCurChar()))
	{
		if (c == '.')
		{
			src.prev();
			const char *s2 = src.getCurStrPointer();
			const char *s1 = filenameWithExt.getCurStrPointer();
			if (s2 >= s1)
				out_result->setup (s1, 0, (u32)(s2-s1) +1 );
			else
				out_result->setup (s1, 0, 0);
			return true;
		}
		src.prev();
	}

	*out_result = filenameWithExt;
	return true;
}

//*****************************************
bool parser::extractFilePathWithSlash	(const parser::Iter &srcIN, parser::Iter *out_result)
{
	parser::Iter src = srcIN;

	src.toLast();
	const char *s2 = src.getCurStrPointer();
	if (NULL == s2)
		return false;

	char c;
	while ((c = src.getCurChar()))
	{
		if (c == '/')
		{
			s2 = src.getCurStrPointer();
			src.toFirst();
			const char *s1 = src.getCurStrPointer();
			assert (s1);
			if (s2 >= s1)
				out_result->setup (s1, 0, (u32)(s2-s1) +1);
			else
				out_result->setup (s1, 0, 0);
			return true;
		}
		src.prev();
	}

	*out_result = srcIN;
	return true;
}

//*****************************************
bool parser::extractFilePathWithOutSlash (const parser::Iter &srcIN, parser::Iter *out_result)
{
	parser::Iter pathWithSlash;
	if (!extractFilePathWithSlash (srcIN, &pathWithSlash))
		return false;

	parser::Iter src = pathWithSlash;
	src.toLast();
	if (src.getCurChar() == '/')
		src.prev();
	
	const char *s1 = pathWithSlash.getCurStrPointer();
	const char *s2 = src.getCurStrPointer();
	if (s2 >= s1)
		out_result->setup (s1, 0, (u32)(s2-s1) +1);
	else
		out_result->setup (s1, 0, 0);
	return true;

}

