#ifndef _rheaIniFile_h_
#define _rheaIniFile_h_
#include "rheaArray.h"
#include "rheaFastArray.h"
#include "rheaString.h"
#include "string/rheaUTF8String.h"


namespace rhea
{
	class IniFileSection;

	/*===============================================
	 * IniFile
	 *
	 */
	class IniFile
	{
	public:
								IniFile ();
								~IniFile();

								//============================ 
		void					setup (rhea::Allocator *alloc)												{ assert(NULL==allocator); allocator=alloc; }
		void					unsetup();
		void					reset();

								//============================ load / save
		void					save ();
		void					saveAs (const u8 * const filename);


								//============================ parse
		bool					loadAndParse (const u8 *filename);
		bool					parseFromMemory (const void *buffer, u32 sizeOfBuffer);

								//============================ set / get
								// identifier usa la notazione "." per indicare le sottosezioni
		void					set (const u8 *identifier, const u8 *value, bool bCreateIfNotFound = true);
		bool					get (const u8 *identifier, utf8::String &out) const;
		bool					get (const u8 *identifier, u8 *out, u32 sizeofout) const;
		void					getOrDefault (const u8 *identifier, const u8 *defaultValue, utf8::String &out) const;
		void					getOrDefault (const u8 *identifier, const u8 *defaultValue, u8 *out, u32 sizeofout) const;
		bool					checkString (const u8 *identifier, const u8 *valueToCmp, bool bSaseSens=false) const;
									//ritorna true se identifier esiste ed è = a valueToCmp
		f32						getOrDefaultAsF32 (const u8 *identifier, f32 defaultValue) const;
		u32						getOrDefaultAsU32 (const u8 *identifier, u32 defaultValue) const;
		i32						getOrDefaultAsI32 (const u8 *identifier, i32 defaultValue) const;
		i32						getOrDefaultHexToI32 (const u8 *identifier, const u8 *defaultValue) const;
									//legge una stringa in hex e ritorna i32 (l'hex NON deve iniziare con 0x)

								//============================ section
		IniFileSection*			getRoot() const																	{ return root; }
		IniFileSection*			getSubsection (const u8 *name) const;
									// name usa la notazione "." per indicare le sottosezioni
		u32						getNSubsection () const;
		IniFileSection*			getSubsectionByIndex (u32 i) const;

	private:
		void					priv_errorMessageNear (const utf8::String &msg, const string::utf8::Iter &src) const;
		bool					priv_Parse_separator_Value (string::utf8::Iter &src, string::utf8::Iter *result, u8 separator) const;
		bool					priv_Parse_Section (IniFileSection *section, string::utf8::Iter &src);
		void					priv_toNextValidChar (IniFileSection *section, string::utf8::Iter &src) const;
		

	private:
		Allocator				*allocator;
		utf8::String			filename;
		IniFileSection			*root;
		u32						nextUIDForSameSection;
	};







	/*===============================================
	 * IniFileSection
	 *
	 */
	class IniFileSection
	{
	public:
								IniFileSection (Allocator *alloc);
								~IniFileSection();

								//============================= set / get
								// identifier usa la notazione "." per indicare le sottosezioni
		void					set (const u8 *identifier, const u8 *value, bool bCreateIfNotFound = true);
		bool					get (const u8 *identifier, utf8::String &out) const;
		bool					get (const u8 *identifier, u8 *out, u32 sizeofout) const;
		void					getOrDefault (const u8 *identifier, const u8 *defaultValue, utf8::String &out) const;
		void					getOrDefault (const u8 *identifier, const u8 *defaultValue, u8 *out, u32 sizeofout) const;
		bool					checkString (const u8 *identifier, const u8 *valueToCmp, bool bCaseSensitive=false) const;
									//ritorna true se identifier esiste ed è = a valueToCmp
		f32						getOrDefaultAsF32 (const u8 *identifier, f32 defaultValue) const;
		u32						getOrDefaultAsU32 (const u8 *identifier, u32 defaultValue) const;
		i32						getOrDefaultAsI32 (const u8 *identifier, i32 defaultValue) const;
		i32						getOrDefaultHexToI32 (const u8 *identifier, const u8 *defaultValue) const;

								//============================= add
		IniFileSection*			addSubsection (const u8 *name);
		void					addComment (const u8 *s, u32 len);
		void					addBlob (const u8 *s, u32 len);
		
								//============================= query
		u32						getNSubsection () const									{ return subSection.getNElem(); }
		IniFileSection*			getSubsectionByIndex (u32 i) const						{ assert (i<getNSubsection()); return subSection.getElem(i); }
		IniFileSection*			getSubsection (const u8 *name) const;
									// name usa la notazione "." per indicare le sottosezioni
		
		u32						getNIdentifier() const									{ return identifier.getNElem(); }
		u32						identifierExists (const u8 *name) const;
		const u8*				getValueByIndex (u32 index) const;
		const u8*				getIdentifierByIndex (u32 index) const;

								//============================= Save
		void					save (FILE *f, u32 tabCount) const;
		
								//============================ var
		utf8::String			name;

	private:
		enum class eElem : u8
		{
			comment				= 0,
			subsection			= 1,
			identifierValue		= 2,
			blob				= 3
		};
		class sElem
		{
		public:
					sElem()	{}
			sElem	operator= (const sElem &b)		{ what=b.what; index=b.index; return *this; }

			eElem	what;
			u32		index;
		};

	private:
		const u8*							priv_get (const u8 *identifier) const;
		void								priv_set (const u8 *identifierName, const u8 *valueIN, u32 valuelen);
		IniFileSection*						priv_simpleSubsectionExists (const u8 *name) const;
												//cerca una subsection di this, senza calcolare evenutali "."

	private:
		Allocator							*allocator;
		Array<sElem>						elements;
		Array<utf8::String>					blob;
		Array<utf8::String>					comments;
		Array<utf8::String>					identifier;
		Array<utf8::String>					value;
		FastArray<IniFileSection*>			subSection;

	friend IniFile;
	};
};
#endif //_rheaIniFile_h_