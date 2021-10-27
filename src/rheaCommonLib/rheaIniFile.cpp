#include "rhea.h"
#include "rheaIniFile.h"

using namespace rhea;


//********************************************
IniFile::IniFile()
{
	allocator = NULL;
	root = NULL;
//	loadBuffer = NULL;
}

//********************************************
IniFile::~IniFile()
{
	unsetup();
}

//********************************************
void IniFile::unsetup()
{
	if (NULL == allocator)
		return;
	RHEADELETE(allocator, root);
	allocator = NULL;
}

//********************************************
void IniFile::reset()
{
	nextUIDForSameSection = 0;
	RHEADELETE(allocator, root);
}

//********************************************
void IniFile::priv_errorMessageNear (const utf8::String &msg UNUSED_PARAM, const string::utf8::Iter &src) const
{
	u8 temp[256];
	src.copyStrFromCurrentPositionToEnd (temp, sizeof(temp));
	//gos::logErr ("%s was expected near %s", msg.getBuffer(), temp);
}

//********************************************
void IniFile::save()
{
	saveAs(filename.getBuffer());
}

//********************************************
void IniFile::saveAs  (const u8 * const filenameIN)
{
	filename = filenameIN;

	FILE *f;
	f = rhea::fs::fileOpenForWriteBinary (filenameIN);
	if (NULL == f)
	{
		DBGBREAK;
		return;
	}

	root->save (f, 0);
    rhea::fs::fileClose(f);
}


//********************************************
bool IniFile::priv_Parse_separator_Value (string::utf8::Iter &src, string::utf8::Iter *result, u8 separator) const
{
	string::utf8::toNextValidChar(src);

	// mi aspetto un carattere separatore
	if (src.getCurChar() != separator)
	{
		utf8::String temp;
		temp = "IniFile::priv_Parse_Identifier_separator_Value () -> '";
		temp.append (&separator, 1);
		temp.append ("'", 1);
		priv_errorMessageNear (temp, src);
		return false;
	}
	src.advanceOneChar();

	//mi aspetto un value
	string::utf8::toNextValidChar(src);

	UTF8Char closer[2] = { UTF8Char('\r'), UTF8Char('\n') };
	if (!string::utf8::extractValue (src, result, closer, 2))
	{
		priv_errorMessageNear ("IniFile::Parse () -> A valid string", src);
		return false;
	}
	return true;
}

//********************************************
bool IniFile::loadAndParse (const u8 *filename)
{
	reset();
	u32 fileSize;
	u8 *buffer = fs::fileCopyInMemory (filename, rhea::getScrapAllocator(), &fileSize);
	if (NULL == buffer)
		return false;

	bool ret = parseFromMemory (buffer, fileSize);
	RHEAFREE(rhea::getScrapAllocator(), buffer);
	return ret;	
}

//********************************************
bool IniFile::parseFromMemory (const void *buffer, u32 sizeOfBuffer)
{
	if (!allocator)
		allocator = rhea::getSysHeapAllocator();

	RHEADELETE(allocator, root);
	root = RHEANEW (allocator, IniFileSection)(allocator);

	string::utf8::Iter src;
	src.setup ((const u8*)buffer, 0, sizeOfBuffer);
	if (!priv_Parse_Section (root, src))
	{
		//gos::logErr ("IniFile::parseFromMemory() -> filename: [%s]\n", filename);
		return false;
	}
	return true;
}


//********************************************
void IniFile::priv_toNextValidChar (IniFileSection *section, string::utf8::Iter &src) const
{
	const UTF8Char	cTabAndBlank[2] = { UTF8Char(' '), UTF8Char('\t') };

	u32 nLine = 0;
	while (!src.getCurChar().isEOF())
	{
		//mi porto sul primo char buono
		string::utf8::skip (src, cTabAndBlank, 2);

		if (src.getCurChar() == '\n' || src.getCurChar() == '\r')
		{
			string::utf8::skipEOL (src);
			++nLine;
		}
		else
			break;
	}

	if (nLine)
	{
		if (nLine > 32)
			nLine = 32;
		u8 eol[32];
		memset (eol, '\n', 32);
		section->addBlob (eol, nLine);
	}
}

//********************************************
bool IniFile::priv_Parse_Section (IniFileSection *section, string::utf8::Iter &src)
{
	string::utf8::Iter result;
	while (!src.getCurChar().isEOF())
	{
		//mi porto sul primo char buono
		priv_toNextValidChar (section, src);
		if (src.getCurChar().isEOF())
			break;

		//qui può esserci un commento
		if (string::utf8::extractCPPComment (src, &result))
		{
			section->addComment (result.getPointerToCurrentPosition(), result.getBytesLeft());
			string::utf8::skipEOL(src);
			continue;
		}

		//oppure la fine della sezione
		if (src.getCurChar() == '}')
		{
			string::utf8::advanceToEOL (src, true);
			break;
		}


		//mi aspetto un identifier
		u8 identifierName[128];
		if (!string::utf8::extractIdentifier (src, &result))
		{
			//ok, in generale questo è un errore ma c'è un caso particolare. Consento alle sezioni di chiamarsi [nomeSezione] ovvero
			//con le parentesi quadre attorno al nome. Questa sintassi vuol dire che posso avere nel file n sezioni con lo stesso nome
			//e che d'ufficio io appendo un numero univoco al nome della sezione durante il parsing
			
			const UTF8Char parentesiQuadraAperta('[');
			const UTF8Char parentesiQuadraChiusa(']');
			bool isErr = true;
			while (1)
			{
				if (src.getCurChar() != parentesiQuadraAperta)
					break;

				src.advanceOneChar();
				if (!string::utf8::extractIdentifier (src, &result, &parentesiQuadraChiusa, 1))
					break;

				//mi assicuro che la ] sia l'ultimo char
				result.copyAllStr (identifierName, sizeof(identifierName));
				u32 n = string::utf8::lengthInBytes(identifierName);
				if (identifierName[n - 1] != ']')
					break;
				identifierName[n - 1] = 0;

				//ok, a questo punto il prossimo valido char deve essere una {
				string::utf8::toNextValidChar (src);
				if (src.getCurChar() != '{')
					break;

				//va bene, abbiamo trovato un nome sezione particolare
				char num[32];
				sprintf_s (num, sizeof(num), "%d", nextUIDForSameSection++);
				string::utf8::concatStr (identifierName, sizeof(identifierName), num);
				IniFileSection *subSection = section->addSubsection (identifierName);
				src.advanceOneChar();
				string::utf8::toNextValidChar(src);
				string::utf8::skipEOL(src);
				if (!priv_Parse_Section (subSection, src))
					return false;

				isErr = false;
				break;
			}
			
			if (isErr)
			{
				priv_errorMessageNear ("IniFile::Parse () -> A valid identifier", src);
				return false;
			}
			else
				continue;
		}
		result.copyAllStr (identifierName, sizeof(identifierName));

		//mi porto sul primo char buono
		string::utf8::toNextValidChar (src);

		//a questo punto o c'è un valore (identifier : valore) oppure l'inizio di una sezione ({)
		if (src.getCurChar() == '{')
		{
			IniFileSection *subSection = section->addSubsection (identifierName);
			src.advanceOneChar();
			string::utf8::toNextValidChar(src);
			string::utf8::skipEOL(src);
			if (!priv_Parse_Section (subSection, src))
				return false;
			continue;
		}
		
		//dato che non è iniziata una sezione, mi aspetto identifier : value
		if (!priv_Parse_separator_Value (src, &result, ':'))
			return false;
		
		section->priv_set (identifierName, result.getPointerToCurrentPosition(), result.getBytesLeft());
		string::utf8::advanceToEOL(src, true);
	}
	return true;
}



//********************************************
bool IniFile::get (const u8 *identifier, utf8::String &out) const
{
	if (NULL == root)
		return false;
	return root->get (identifier, out);
}

//********************************************
bool IniFile::get (const u8 *identifier, u8 *out, u32 sizeofout) const
{
	if (NULL == root)
		return false;
	return root->get (identifier, out, sizeofout);
}

//********************************************
void IniFile::getOrDefault (const u8 *identifier, const u8 *defaultValue, utf8::String &out) const
{
	if (NULL == root)
		out = defaultValue;
	else
		root->getOrDefault (identifier, defaultValue, out);
}

//*******************************************
void IniFile::getOrDefault (const u8 *identifier, const u8 *defaultValue, u8 *out, u32 sizeofout) const
{
	if (NULL == root)
	{
		assert (NULL != defaultValue);
		u32 n = string::utf8::lengthInBytes(defaultValue);
		if (n>=sizeofout)
		{
			n = sizeofout-1;
			DBGBREAK;
		}
		memcpy (out, defaultValue, n);
		out[n] = 0;
	}
	else
		root->getOrDefault (identifier, defaultValue, out, sizeofout);
}

//********************************************
bool IniFile::checkString (const u8 *identifier, const u8 *valueToCmp, bool bCaseSens) const
{
	if (NULL == root)
		return false;
	return root->checkString (identifier, valueToCmp, bCaseSens);
}

//********************************************
f32 IniFile::getOrDefaultAsF32 (const u8 *identifier, f32 defaultValue) const
{
	if (NULL == root)
		return defaultValue;
	return root->getOrDefaultAsF32 (identifier, defaultValue);
}

//********************************************
u32 IniFile::getOrDefaultAsU32 (const u8 *identifier, u32 defaultValue) const
{
	if (NULL == root)
		return defaultValue;
	return root->getOrDefaultAsU32 (identifier, defaultValue);
}

//********************************************
i32 IniFile::getOrDefaultAsI32 (const u8 *identifier, i32 defaultValue) const
{
	if (NULL == root)
		return defaultValue;
	return root->getOrDefaultAsI32 (identifier, defaultValue);
}

//********************************************
i32 IniFile::getOrDefaultHexToI32 (const u8 *identifier, const u8 *defaultValue) const
{
	if (NULL == root)
	{
		u32 out;
		string::ansi::hexToInt ((const char*)defaultValue, &out);
		return (i32)out;
	}
	return root->getOrDefaultHexToI32 (identifier, defaultValue);
}

//********************************************
void IniFile::set (const u8 *identifier, const u8 *value, bool bCreateIfNotFound)
{
	if (NULL == root)
		root = RHEANEW(allocator, IniFileSection)(allocator);
	root->set (identifier, value, bCreateIfNotFound);
}

//********************************************
IniFileSection*	IniFile::getSubsection (const u8 *name) const
{ 
	if (NULL==root) 
		return NULL; 
	return root->getSubsection(name); 
}

//********************************************
u32 IniFile::getNSubsection () const
{
	if (NULL == root)
		return 0;
	return root->getNSubsection();
}

//********************************************
IniFileSection*	IniFile::getSubsectionByIndex (u32 i) const
{
	if (NULL == root)
		return NULL;
	return root->getSubsectionByIndex(i);
}
