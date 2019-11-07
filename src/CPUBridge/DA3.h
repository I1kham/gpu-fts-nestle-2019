#ifndef _DA3_h_
#define _DA3_h_
#include "../rheaCommonLib/rhea.h"


/****************************************************
 *
 */
class DA3
{
public:
					DA3();
					~DA3()																						{ free(); }

	bool			loadInMemory (rhea::Allocator *allocator, const char *fullFilePathAndName);
	void			save(const char *fullFilePathAndName);
	void			free();

	u8				readU8(u32 location) const;
	void			writeU8(u32 location, u8 value);

	u16				readU16(u32 location) const;
	void			writeU16(u32 location, u16 value);

private:
	rhea::Allocator *allocator;
	u8				*blob;
	u32				sizeOfBlob;
};


#endif // _DA3_h_
