#include "rhea.h"
#include "rheaIniFile.h"


using namespace rhea;


//********************************************
IniFileSection::IniFileSection (Allocator *alloc)
{
	assert (alloc);
	allocator = alloc;
	name.setAllocator (allocator);
	subSection.setup (alloc, 4);
	identifier.setup (alloc, 32);
	comments.setup (alloc, 32);
	blob.setup (alloc, 128);
	elements.setup (alloc, 128);
	value.setup (alloc, 64);
}

//********************************************
IniFileSection::~IniFileSection()
{
	u32 n = subSection.getNElem();
	for (u32 i=0; i<n; i++)
		RHEADELETE(allocator, subSection[i]);
}

//********************************************
IniFileSection* IniFileSection::priv_simpleSubsectionExists (const u8 *name) const
{
	u32 n = subSection.getNElem();
	for (u32 i=0; i<n; i++)
	{
		if (subSection(i)->name.isEqualTo (name, false))
			return subSection.getElem(i);
	}
	return NULL;
}

//********************************************
IniFileSection* IniFileSection::addSubsection (const u8 *name)
{
	IniFileSection *ret = priv_simpleSubsectionExists (name);
	if (NULL == ret)
	{
		ret = RHEANEW(allocator, IniFileSection)(allocator);
		ret->name = name;
		u32 n = subSection.getNElem();
		subSection[n] = ret;

		u32 n2 = elements.getNElem();
		elements[n2].what = eElem::subsection;
		elements[n2].index = n;
	}
	return ret;
}

//********************************************
u32 IniFileSection::identifierExists (const u8 *name) const
{
	u32 n = identifier.getNElem();
	for (u32 i=0; i<n; i++)
	{
		if (identifier(i).isEqualTo (name, false))
			return i;
	}
	return u32MAX;
}

//********************************************
void IniFileSection::set (const u8 *identifierName, const u8 *valueIN, bool bCreateIfNotFound)
{
	assert (NULL != identifierName && NULL != allocator);
	
	const UTF8Char cPunto('.');
	rhea::string::utf8::Iter src;
	src.setup (identifierName, 0, (u32)string::utf8::lengthInBytes(identifierName));
	
	if (!string::utf8::advanceUntil (src, &cPunto, 1))
	{
		//non ho trovato il "."
		u32 index = identifierExists (identifierName);
		if (u32MAX != index)
			value[index].setFrom (valueIN, (u32)string::utf8::lengthInBytes(valueIN));
		else
		{
			if (bCreateIfNotFound)
			{
				index = identifier.getNElem();
				identifier[index] = identifierName;

				u32 n2 = elements.getNElem();
				elements[n2].what = eElem::identifierValue;
				elements[n2].index = index;
				value[index].setFrom (valueIN, (u32)string::utf8::lengthInBytes(valueIN));
			}
		}
		return;
	}
	else
	{
		//ho trovato il "."
		//estraggo il nome della sezione
		u8 subSectionName[128];
		src.copyStrFromXToCurrentPosition (0, subSectionName, sizeof(subSectionName), false);
		src.advanceOneChar(); //skippo il "."
		
		//la cerco (eventualmente la creo)
		IniFileSection *subSection = priv_simpleSubsectionExists (subSectionName);
		if (NULL == subSection)
		{
			if (bCreateIfNotFound)
				subSection = addSubsection (subSectionName);
			else
				return;
		}

		subSection->set (src.getPointerToCurrentPosition(), valueIN, bCreateIfNotFound);
	}

}

//********************************************
void IniFileSection::priv_set (const u8 *identifierName, const u8 *valueIN, u32 valuelen)
{
	u32 i = identifierExists (identifierName);
	if (u32MAX == i)
	{
		i = identifier.getNElem();
		identifier[i] = identifierName;

		u32 n2 = elements.getNElem();
		elements[n2].what = eElem::identifierValue;
		elements[n2].index = i;
	}

	value[i].setFrom (valueIN, valuelen);
}

//********************************************
void IniFileSection::addComment (const u8 *c, u32 len)
{
	if (NULL == c || len == 0)
		return;

	u32 n = comments.getNElem();
	comments[n].setFrom (c, len);
		
	u32 n2 = elements.getNElem();
	elements[n2].what = eElem::comment;
	elements[n2].index = n;
}

//********************************************
void IniFileSection::addBlob (const u8 *c, u32 len)
{
	if (NULL == c || len == 0)
		return;

	u32 n = blob.getNElem();
	blob[n].setFrom (c, len);
		
	u32 n2 = elements.getNElem();
	elements[n2].what = eElem::blob;
	elements[n2].index = n;
}

//********************************************
void IniFileSection::save (FILE *f, u32 tabCount) const
{
	const u8 oneTab[2] = { "\t" };
	const u8 oneGraffaOpen[2] = { "{" };
	const u8 oneGraffaClose[2] = { "}" };
	const u8 spazioDuepuntiSpazio[4] = {" : "};
	const u8 EOL[4] = { "\r\n" };
	const UTF8Char apiciDoppi("\"");
	
	u8 tabs[128];
	memset (tabs, oneTab[0], sizeof(tabs));
	
#define BW_WRITE_EOL						rhea::fs::fileWrite (f, EOL, 2);
#define BW_WRITE_TABS						rhea::fs::fileWrite (f, tabs, tabCount);
#define BW_WRITE_GRAFFA_OPEN				rhea::fs::fileWrite (f, oneGraffaOpen, 1);
#define BW_WRITE_GRAFFA_CLOSE				rhea::fs::fileWrite (f, oneGraffaClose, 1);
#define BW_WRITE_GRAFFA_TAB					rhea::fs::fileWrite (f, oneTab, 1);
#define BW_WRITE_SPAZIO_DUEPUNTI_SPAZIO		rhea::fs::fileWrite (f, spazioDuepuntiSpazio, 3);
	
	if (name.lengthInBytes())
	{
		BW_WRITE_TABS
		rhea::fs::fileWrite (f, name.getBuffer(), name.lengthInBytes());
		BW_WRITE_EOL
		BW_WRITE_TABS
		BW_WRITE_GRAFFA_OPEN
		BW_WRITE_EOL
		++tabCount;
	}
		
	u32 n2 = elements.getNElem();
	for (u32 i2=0; i2<n2; i2++)
	{
		switch (elements(i2).what)
		{
		default:
			DBGBREAK;
			break;

		case eElem::blob:
			rhea::fs::fileWrite (f, blob(elements(i2).index).getBuffer(), blob(elements(i2).index).lengthInBytes());
			break;

		case eElem::comment:
			BW_WRITE_TABS
			rhea::fs::fileWrite (f, comments(elements(i2).index).getBuffer(), comments(elements(i2).index).lengthInBytes());
			BW_WRITE_EOL
			break;

		case eElem::subsection:
			subSection(elements(i2).index)->save (f, tabCount);
			break;

		case eElem::identifierValue:
			{
				u32 i = elements(i2).index;

				//se in value non ci sono spazi, lo salvo cosi' com'è
				if (-1 == value(i).findFirst(' '))
				{
					BW_WRITE_TABS
					rhea::fs::fileWrite (f, identifier(i).getBuffer(), identifier(i).lengthInBytes());
					BW_WRITE_SPAZIO_DUEPUNTI_SPAZIO
					rhea::fs::fileWrite (f, value(i).getBuffer(), value(i).lengthInBytes());
					BW_WRITE_EOL
				}
				else
				{
					//se ci sono spazi, lo salvo racchiuso tra doppi apici o singoli apici
					BW_WRITE_TABS
					rhea::fs::fileWrite (f, identifier(i).getBuffer(), identifier(i).lengthInBytes());
					BW_WRITE_SPAZIO_DUEPUNTI_SPAZIO
					
					u8 c[2] = { '\'', 0 };
					if (-1 == value(i).findFirst(apiciDoppi))
						c[0] = '"';
						
					rhea::fs::fileWrite (f, c, 1);
					rhea::fs::fileWrite (f, value(i).getBuffer(), value(i).lengthInBytes());
					rhea::fs::fileWrite (f, c, 1);
					BW_WRITE_EOL
				}
			}
			break;
		}
	}
	

	if (name.lengthInBytes())
	{
		rhea::fs::fileWrite (f, tabs, tabCount-1);
		BW_WRITE_GRAFFA_CLOSE
		BW_WRITE_EOL
		BW_WRITE_EOL
		BW_WRITE_EOL
	}

#undef BW_WRITE_EOL
#undef BW_WRITE_TABS
#undef BW_WRITE_GRAFFA_OPEN
#undef BW_WRITE_GRAFFA_CLOSE
#undef BW_WRITE_GRAFFA_TAB
#undef BW_WRITE_SPAZIO_DUEPUNTI_SPAZIO
}

//********************************************
const u8 *IniFileSection::getValueByIndex (u32 index) const
{
	if (index < value.getNElem())
		return value(index).getBuffer();
	else
	{
		DBGBREAK;
		return NULL;
	}	
}

//********************************************
const u8 *IniFileSection::getIdentifierByIndex (u32 index) const
{
	if (index < identifier.getNElem())
		return identifier(index).getBuffer();
	else
	{
		DBGBREAK;
		return NULL;
	}	
}

//********************************************
const u8 *IniFileSection::priv_get (const u8 *identifier) const
{
	assert (NULL != identifier);
	
	rhea::string::utf8::Iter src;
	src.setup (identifier, 0, (u32)string::utf8::lengthInBytes(identifier));
		
	const UTF8Char cPunto('.');
	if (!string::utf8::advanceUntil (src, &cPunto, 1))
	{
		//non ho trovato il "."
		u32 index = identifierExists (identifier);
		if (u32MAX == index)
			return NULL;
		const u8 *ret = getValueByIndex (index);
		assert (NULL != ret);
		return ret;
	}
	else
	{
		//ho trovato il "."
		u8 subSectionName[128];
		src.copyStrFromXToCurrentPosition(0, subSectionName, sizeof(subSectionName), false);
		src.advanceOneChar(); //skippo il "."
		
		//la cerco
		IniFileSection *subSection = priv_simpleSubsectionExists (subSectionName);
		if (NULL == subSection)
			return NULL;

		return subSection->priv_get (src.getPointerToCurrentPosition());
	}
}

//********************************************
bool IniFileSection::get (const u8 *identifier, utf8::String &out) const
{
	const u8 *pstr = priv_get (identifier);
	if (NULL == pstr)
		return false;
	out = pstr;
	return true;
}

//********************************************
bool IniFileSection::get (const u8 *identifier, u8 *out, u32 sizeofout) const
{
	const u8 *pstr = priv_get (identifier);
	if (NULL == pstr)
		return false;

	u32 n = (u32)string::utf8::lengthInBytes(pstr);
	if (n>=sizeofout)
	{
		n = sizeofout-1;
		DBGBREAK;
	}
	memcpy (out, pstr, n);
	out[n] = 0;

	return true;
}

//********************************************
void IniFileSection::getOrDefault (const u8 *identifier, const u8 *defaultValue, utf8::String &out) const
{
	if (get(identifier, out))
		return;
	out = defaultValue;
}

//*******************************************
void IniFileSection::getOrDefault (const u8 *identifier, const u8 *defaultValue, u8 *out, u32 sizeofout) const
{
	if (get (identifier, out, sizeofout))
		return;
	
	assert (NULL != defaultValue);
	u32 n = (u32)string::utf8::lengthInBytes(defaultValue);
	if (n>=sizeofout)
	{
		n = sizeofout-1;
		DBGBREAK;
	}
	memcpy (out, defaultValue, n);
	out[n] = 0;
}

//********************************************
bool IniFileSection::checkString (const u8 *identifier, const u8 *valueToCmp, bool bCaseSensitive) const
{
	u8 out[256];
	if (!get (identifier, out, sizeof(out)))
		return false;
	return string::utf8::areEqual (out, valueToCmp, bCaseSensitive);
}

//********************************************
f32 IniFileSection::getOrDefaultAsF32 (const u8 *identifier, f32 defaultValue) const
{
	const u8 *s = priv_get(identifier);
	if (!s)
		return defaultValue;
	return string::utf8::toF32(s);
}

//********************************************
u32 IniFileSection::getOrDefaultAsU32 (const u8 *identifier, u32 defaultValue) const
{
	const u8 *s = priv_get(identifier);
	if (!s)
		return defaultValue;
	return (u32)string::utf8::toI32(s);
}

//********************************************
i32 IniFileSection::getOrDefaultAsI32 (const u8 *identifier, i32 defaultValue) const
{
	const u8 *s = priv_get(identifier);
	if (!s)
		return defaultValue;
	return string::utf8::toI32(s);
}

//********************************************
i32 IniFileSection::getOrDefaultHexToI32 (const u8 *identifier, const u8 *defaultValue) const
{
	u32 out;
	const u8 *s = priv_get(identifier);
	if (!s)
		string::ansi::hexToInt ((const char*)defaultValue, &out, (u32)string::utf8::lengthInBytes(defaultValue));
	else		
		string::ansi::hexToInt ((const char*)s, &out, (u32)string::utf8::lengthInBytes(s));
	return (i32)out;
}

//********************************************
IniFileSection* IniFileSection::getSubsection (const u8 *name) const
{
	if (NULL==name || (NULL!=name && name[0]==0x00))
		return NULL;
	
	rhea::string::utf8::Iter src;
	src.setup (name, 0, (u32)string::utf8::lengthInBytes(name));
	
	const UTF8Char cPunto('.');
	if (!string::utf8::advanceUntil (src, &cPunto, 1))
	{
		//non c'è il punto
		return this->priv_simpleSubsectionExists (name);
	}

	//estraggo il nome della sezione
	u8 subSectionName[128];
	src.copyStrFromXToCurrentPosition (0, subSectionName, sizeof(subSectionName), false);
	src.advanceOneChar(); //skippo il punto

	//la cerco
	IniFileSection *subSection = priv_simpleSubsectionExists (subSectionName);
	if (NULL == subSection)
		return NULL;
	return subSection->getSubsection (src.getPointerToCurrentPosition());
}