#include "winTerminal.h"

void debug_transalteLastError(DWORD err)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	DBGBREAK;
	LocalFree(lpMsgBuf);
}

#define INPUT_BUFFER_ATTRIBUTE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) | BACKGROUND_GREEN | FOREGROUND_INTENSITY
#define VIDEO_BUFFER_ATTRIBUTE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define HEADER_BUFFER_ATTRIBUTE (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) | BACKGROUND_BLUE | FOREGROUND_INTENSITY

//**********************************************
WinTerminal::WinTerminal()
{
	rhea::criticalsection::init(&cs);
	hConsoleBuffer1 = INVALID_HANDLE_VALUE;

	videoBufferForInput.alloc (1, NUM_MAX_COLS);
	videoBufferForOutput.alloc (60, NUM_MAX_COLS);
	videoBufferForHeader.alloc (HEADER_HEIGHT, NUM_MAX_COLS);

	screenCols = screenRows = 0;
	cursorX = 0;
	cursorY = 0;
	bodyY = bodyFirstVisibleRow = 0;
	cursorIsFull = 0;
	timeToBlinkCursor = 1;
	inputLineNumber = 1;

	indent = isANewLine = 0;
	priv_buildIndentStr();
}

//**********************************************
WinTerminal::~WinTerminal()
{
	rhea::criticalsection::close(cs);
}

//**********************************************
void WinTerminal::priv_clearInputBuffer()
{
	for (u16 i = 0; i < videoBufferForInput.getNumCols(); i++)
	{
		videoBufferForInput.buffer[i].Attributes = INPUT_BUFFER_ATTRIBUTE;
		videoBufferForInput.buffer[i].Char.AsciiChar = 0;
	}
	cursorX = 0;
}

//**********************************************
void WinTerminal::priv_appendStringToBody (const char *prefix, const char *sIN, WORD textAndBgAttribute)
{
	rhea::criticalsection::enter(cs);

	if (bodyY >= videoBufferForOutput.getNumRows())
	{
		//discardo un po' di linee dal buffer
		const u16 DISCARD_DIVIDER = 4;
		u16 numRowToDiscard = videoBufferForOutput.getNumRows() / DISCARD_DIVIDER;
		if (numRowToDiscard == 0)
			numRowToDiscard = 1;

		u32 ct = numRowToDiscard * videoBufferForOutput.getNumCols();
		const u16 numRowToCopy = videoBufferForOutput.getNumRows() - numRowToDiscard;
		memcpy(videoBufferForOutput.buffer, &videoBufferForOutput.buffer[ct], numRowToCopy * videoBufferForOutput.getNumCols() * sizeof(CHAR_INFO));

		ct = numRowToCopy * videoBufferForOutput.getNumCols();
		for (u16 y = 0; y < numRowToDiscard; y++)
		{
			for (u16 x = 0; x < videoBufferForOutput.getNumCols(); x++)
			{
				videoBufferForOutput.buffer[ct].Attributes = VIDEO_BUFFER_ATTRIBUTE;
				videoBufferForOutput.buffer[ct].Char.AsciiChar = ' ';
				++ct;
			}
		}

		bodyFirstVisibleRow = 0;
		bodyY = numRowToCopy;
	}



	char s[256];
	if (NULL == prefix)
		sprintf_s(s, sizeof(s), "%s", sIN);
	else
		sprintf_s(s, sizeof(s), "%s%s", prefix, sIN);
	inputLineNumber++;


	u32 dstCT = videoBufferForOutput.getNumCols() * bodyY;
	++bodyY;

	u16 i;
	for (i = 0; i < videoBufferForOutput.getNumCols(); i++)
	{
		videoBufferForOutput.buffer[dstCT].Attributes = textAndBgAttribute;
		if (s[i] == 0x00)
		{
			videoBufferForOutput.buffer[dstCT++].Char.AsciiChar = ' ';
			break;
		}
		videoBufferForOutput.buffer[dstCT++].Char.AsciiChar = s[i];
	}

	for (;i < videoBufferForInput.getNumCols(); i++)
	{
		videoBufferForOutput.buffer[dstCT].Attributes = VIDEO_BUFFER_ATTRIBUTE;
		videoBufferForOutput.buffer[dstCT].Char.AsciiChar = ' ';
		dstCT++;
	}


	//porto in vista la riga appena inserita
	const u16 body_height = priv_getBodyH();
	if (bodyY - bodyFirstVisibleRow > body_height)
		bodyFirstVisibleRow = bodyY - body_height;

	rhea::criticalsection::leave(cs);
}

//**********************************************
void WinTerminal::priv_onKeyEvent(const KEY_EVENT_RECORD *ev)
{
	if (!ev->bKeyDown)
		return;

	const char ascii = ev->uChar.AsciiChar;
	if ( (ascii>='a' && ascii<='z') || (ascii >= 'A' && ascii <= 'Z') || (ascii >= '0' && ascii <= '9') || ascii ==' ' || ascii == '.' || ascii == '?' || ascii == '-' || ascii == ':' || ascii == '_')
	{
		for (u32 i = 0; i < ev->wRepeatCount; i++)
		{
			videoBufferForInput.put(cursorX, 0, ascii);
			++cursorX;
		}
		return;
	}

	switch (ascii)
	{
	case 8:	//DEL
		if (cursorX)
		{
			cursorX--;
			videoBufferForInput.put(cursorX, 0, (char)0x00);
		}
		break;

	case 13: //enter
		{
			char	s[256];
			u16		i;

			for (i = 0; i < videoBufferForInput.getNumCols(); i++)
			{
				s[i] = videoBufferForInput.buffer[i].Char.AsciiChar;
				if (s[i] == 0x00)
					break;
			}
			s[i] = 0x00;
			priv_appendStringToBody(NULL, s, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			priv_clearInputBuffer();
			virt_onUserCommand(s);
		}
		break;
	}

	switch (ev->wVirtualKeyCode)
	{
	case VK_UP:	//freccia su
		if (bodyFirstVisibleRow)
			bodyFirstVisibleRow--;
		break;

	case VK_DOWN:
		if (bodyFirstVisibleRow + priv_getBodyH() < bodyY)
			bodyFirstVisibleRow++;
		break;
	}
}

//**********************************************
void WinTerminal::priv_onResizeEvent(const WINDOW_BUFFER_SIZE_RECORD *ev)
{
	//printf("Console screen buffer is %d columns by %d rows.\n", ev->dwSize.X, ev->dwSize.Y);
	screenRows = ev->dwSize.Y;
	screenCols = ev->dwSize.X;
	cursorY = screenRows - 1;
}

//**********************************************
void WinTerminal::priv_render()
{
	if (screenRows < 1)
		return;

	rhea::criticalsection::enter(cs);

	if (hCurConsoleBuffer == hConsoleBuffer1)
		hCurConsoleBuffer = hConsoleBuffer2;
	else
		hCurConsoleBuffer = hConsoleBuffer1;


	COORD srcStart;
	COORD srcSize;
	SMALL_RECT dstRect;


	//header rendering
	srcStart.X = 0;
	srcStart.Y = 0;
	
	srcSize.X = videoBufferForHeader.getNumCols();
	srcSize.Y = HEADER_HEIGHT;
	
	dstRect.Top = HEADER_STARTY;
	dstRect.Left = 0;
	dstRect.Bottom = HEADER_STARTY + HEADER_HEIGHT - 1;
	dstRect.Right = srcSize.X - 1;

	WriteConsoleOutput(
		hCurConsoleBuffer, // screen buffer to write to 
		videoBufferForHeader.buffer,			// buffer to copy from 
		srcSize,			// col-row size of chiBuffer 
		srcStart,			// top left src cell in chiBuffer 
		&dstRect);



	//Body rendering
	srcStart.X = 0;
	srcStart.Y = 0;
	
	const u16 body_height = priv_getBodyH();
	srcSize.X = videoBufferForOutput.getNumCols();
	srcSize.Y = body_height;
		
	dstRect.Top = BODY_STARTY;
	dstRect.Left = 0;
	dstRect.Bottom = BODY_STARTY +body_height -1;
	dstRect.Right = srcSize.X - 1;

	if (0 == WriteConsoleOutput(
		hCurConsoleBuffer, // screen buffer to write to 
		&videoBufferForOutput.buffer[bodyFirstVisibleRow*videoBufferForOutput.getNumCols()],			// buffer to copy from 
		srcSize,			// col-row size of chiBuffer 
		srcStart,			// top left src cell in chiBuffer 
		&dstRect))
	{
		debug_transalteLastError(GetLastError());
	}



	//input
	srcStart.X = 0;
	srcStart.Y = 0;

	srcSize.X = videoBufferForInput.getNumCols();;
	srcSize.Y = 1;

	dstRect.Top = cursorY;
	dstRect.Left = 0;
	dstRect.Bottom = cursorY;
	dstRect.Right = screenCols -1;

	//devo visualizzare cursore pieno o vuoto?
	--timeToBlinkCursor;
	if (timeToBlinkCursor == 0)
	{
		timeToBlinkCursor = CURSOR_BLINK_TIME;
		cursorIsFull = 1 - cursorIsFull;
	}

	if (cursorIsFull)
		videoBufferForInput.put(cursorX, 0, '_');
		 
	if (0 == WriteConsoleOutput(
		hCurConsoleBuffer, // screen buffer to write to 
		videoBufferForInput.buffer,			// buffer to copy from 
		srcSize,			// col-row size of chiBuffer 
		srcStart,			// top left src cell in chiBuffer 
		&dstRect))
	{
		debug_transalteLastError(GetLastError());
	}
	
	if (cursorIsFull)
		videoBufferForInput.put(cursorX, 0, (char)0x00);

	SetConsoleActiveScreenBuffer(hCurConsoleBuffer);

	rhea::criticalsection::leave(cs);
}


//**********************************************
void WinTerminal::setup()
{
	setHeader("");

	hstdOUT = GetStdHandle(STD_OUTPUT_HANDLE);

	hStdIN = GetStdHandle(STD_INPUT_HANDLE);
	{
		DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		SetConsoleMode(hStdIN, fdwMode);
	}


	hConsoleBuffer1 = CreateConsoleScreenBuffer(
		GENERIC_READ |           // read/write access 
		GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,        // shared 
		NULL,                    // default security attributes 
		CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
		NULL);

	hConsoleBuffer2 = CreateConsoleScreenBuffer(
		GENERIC_READ |           // read/write access 
		GENERIC_WRITE,
		FILE_SHARE_READ |
		FILE_SHARE_WRITE,        // shared 
		NULL,                    // default security attributes 
		CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
		NULL);

	//dimensione del cursore
	CONSOLE_CURSOR_INFO cursorInfo;
	cursorInfo.dwSize = 1; 
	cursorInfo.bVisible = FALSE;
	
	SetConsoleCursorInfo(hConsoleBuffer1, &cursorInfo);
	SetConsoleCursorInfo(hConsoleBuffer2, &cursorInfo);
	SetConsoleCursorInfo(hstdOUT, &cursorInfo);
	SetConsoleCursorInfo(hStdIN, &cursorInfo);
	
	priv_clearInputBuffer();

	hCurConsoleBuffer = hConsoleBuffer1;
	SetConsoleActiveScreenBuffer(hCurConsoleBuffer);
}

//**********************************************
void WinTerminal::setHeader(const char *s)
{
	u32 ct = 0;
	for (u16 y = 0; y < videoBufferForHeader.getNumRows(); y++)
	{
		for (u16 x = 0; x < videoBufferForHeader.getNumCols(); x++)
		{
			videoBufferForHeader.buffer[ct].Attributes = HEADER_BUFFER_ATTRIBUTE;
			videoBufferForHeader.buffer[ct].Char.AsciiChar = 0x00;
			ct++;
		}
	}

	if (NULL == s)
		return;
	if (s[0] == 0x00)
		return;
	videoBufferForHeader.put(0, 0, s);
}

//**********************************************
void WinTerminal::loop()
{
	bQuitLoop = false;
	while (bQuitLoop==false)
	{
		priv_render();

		DWORD nEvents = 0;
		GetNumberOfConsoleInputEvents(hStdIN, &nEvents);
		if (nEvents)
		{
			INPUT_RECORD eventBuf[128];
			ReadConsoleInput(hStdIN, eventBuf, 128, &nEvents);

			for (DWORD i = 0; i < nEvents; i++)
			{
				switch (eventBuf[i].EventType)
				{
				case KEY_EVENT: // keyboard input 
					priv_onKeyEvent(&eventBuf[i].Event.KeyEvent);
					break;


				case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
					priv_onResizeEvent(&eventBuf[i].Event.WindowBufferSizeEvent);
					break;

				case MOUSE_EVENT: // mouse input 
					//MouseEventProc(eventBuf[i].Event.MouseEvent);
					break;

				case FOCUS_EVENT:  // disregard focus events 
				case MENU_EVENT:   // disregard menu events 
					break;
				}
			}
		}

		rhea::thread::sleepMSec(50);
	}
}

//*************************************************
void WinTerminal::cls()
{
	rhea::criticalsection::enter(cs);
	u32 ct = 0;
	for (u16 y = 0; y < videoBufferForOutput.getNumRows(); y++)
	{
		for (u16 i = 0; i < videoBufferForOutput.getNumCols(); i++)
		{
			videoBufferForOutput.buffer[ct].Attributes = VIDEO_BUFFER_ATTRIBUTE;
			videoBufferForOutput.buffer[ct].Char.AsciiChar = ' ';
			ct++;
		}
	}
	bodyFirstVisibleRow = bodyY = 0;
	rhea::criticalsection::leave(cs);
}

//*************************************************
void WinTerminal::priv_buildIndentStr()
{
	u32 n = indent * 2;
	if (n > MAX_INDENT_CHAR)
		n = MAX_INDENT_CHAR;

	memset(strIndent, ' ', n);
	strIndent[n] = 0;
}


//*************************************************
void WinTerminal::incIndent()
{
	rhea::criticalsection::enter(cs);
	++indent;
	priv_buildIndentStr();
	rhea::criticalsection::leave(cs);
}

//*************************************************
void WinTerminal::decIndent()
{
	rhea::criticalsection::enter(cs);
	if (indent)
		--indent;
	priv_buildIndentStr();
	rhea::criticalsection::leave(cs);
}


//*************************************************
void WinTerminal::priv_out(const char *what, u16 colorAttribute)
{
	if (isANewLine)
	{
		priv_appendStringToBody(strIndent, what, colorAttribute);
		isANewLine = 0;
	}
	else
		priv_appendStringToBody(NULL, what, colorAttribute);
}

//*************************************************
void WinTerminal::log(const char *format, ...)
{
	rhea::criticalsection::enter(cs);

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer, INTERNAL_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	priv_outTextCurrentBuffer (VIDEO_BUFFER_ATTRIBUTE);

	rhea::criticalsection::leave(cs);
}

//*************************************************
void WinTerminal::outText(bool red, bool green, bool blue, const char *format, ...)
{
	if (NULL == format)
		return;
	if (format[0] == 0x00)
		return;


	rhea::criticalsection::enter(cs);
	
	va_list argptr;
	va_start(argptr, format);
	vsnprintf(buffer, INTERNAL_BUFFER_SIZE, format, argptr);
	va_end(argptr);

	u16 colorAttribute = 0;
	if (red) colorAttribute |= FOREGROUND_RED;
	if (green) colorAttribute |= FOREGROUND_GREEN;
	if (blue) colorAttribute |= FOREGROUND_BLUE;
	
	priv_outTextCurrentBuffer(colorAttribute);

	rhea::criticalsection::leave(cs);
}

//*************************************************
void WinTerminal::priv_outTextCurrentBuffer(u16 colorAttribute)
{
	u32 n = strlen(buffer);
	if (n == 0)
		return;


	u32 i = 0;
	if (buffer[0] == '\n')
	{
		//priv_appendStringToBody (NULL, "\n");
		isANewLine = 1;
		i++;
	}

	u32 iStart = i;
	while (1)
	{
		if (buffer[i] == '\n')
		{
			buffer[i] = 0;
			if (i - iStart)
				priv_out(&buffer[iStart], colorAttribute);
			//priv_appendStringToBody(NULL, "\n");

			isANewLine = 1;
			i++;
			iStart = i;
			continue;
		}

		if (buffer[i] == 0x00)
		{
			if (i - iStart)
				priv_out(&buffer[iStart], colorAttribute);
			break;
		}

		++i;
	}
}