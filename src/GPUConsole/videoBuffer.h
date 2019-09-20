#ifndef _videoBuffer_h_
#define _videoBuffer_h_
#include "../rheaCommonLib/rhea.h"


/***************************************************
 * VideoBuffer
 *
 */
class VideoBuffer
{
public:
					VideoBuffer();
					~VideoBuffer();

	void			alloc (u16 numRows, u16 numCols);
	void			put (u16 x, u16 y, char c);
	void			put (u16 x, u16 y, const char *s);

	const u16		getNumRows() const									{ return dimy;  }
	const u16		getNumCols() const									{ return dimx; }

	CHAR_INFO		*buffer;

private:
	rhea::Allocator	*allocator;
	u16				dimx, dimy;
};




#endif //_videoBuffer_h_
