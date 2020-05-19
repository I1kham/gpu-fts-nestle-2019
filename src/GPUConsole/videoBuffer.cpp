#include "videoBuffer.h"


//**********************************************
VideoBuffer::VideoBuffer()
{
	allocator = rhea::getScrapAllocator();

	buffer = NULL;
	dimx = dimy = 0;
}

//**********************************************
VideoBuffer::~VideoBuffer()
{
	if (buffer)
		RHEAFREE(allocator, buffer);
}

//**********************************************
void VideoBuffer::alloc (u16 numRows, u16 numCols)
{
	dimx = numCols;
	dimy = numRows;

	if (buffer)
		RHEAFREE(allocator, buffer);
	buffer = (CHAR_INFO*)RHEAALLOC (allocator, sizeof(CHAR_INFO) *dimx*dimy);

	u32 ct = 0;
	for (u16 y = 0; y < dimy; y++)
	{
		for (u16 x = 0; x < dimx; x++)
		{
			buffer[ct].Char.UnicodeChar = 0;
			buffer[ct].Attributes = (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
			ct++;
		}
	}
}

//**********************************************
void VideoBuffer::put (u16 x, u16 y, char c)
{
	assert(x < dimx && y < dimy);
	const u32 ct = (y * dimx) + x;
	//buffer[ct].Char.AsciiChar = c;
	buffer[ct].Char.UnicodeChar = c;
}

//**********************************************
void VideoBuffer::put(u16 x, u16 y, const char *s)
{
	assert(x < dimx && y < dimy);
	const u32 n = (u32)strlen(s);

	u32 ct = (y * dimx) + x;
	for (u32 i = 0; i < n; i++)
	{
		//buffer[ct++].Char.AsciiChar = s[i];
		buffer[ct++].Char.UnicodeChar = s[i];
		if (s[i] == '\n')
			ct += dimx;
	}

}
