#ifndef _winTerminal_h_
#define _winTerminal_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/StdoutLogger.h"
#include "videoBuffer.h"

/***************************************************
 * WinTerminal
 *
 */
class WinTerminal : public rhea::ISimpleLogger
{
public:
						WinTerminal ();
	virtual				~WinTerminal();


	void				setup ();
	void				setHeader(const char *s);

	void				loop();
	void				exitLoop()															{ bQuitLoop = true; }
	void				cls();
	void				outText (bool red, bool green, bool blue, const char *format, ...);


						/* ereditate da ISImpleLogger */
	void				incIndent();
	void				decIndent();
	void				log(const char *format, ...);


	virtual void		virt_onUserCommand(const char *s) {}
						/* chiamata quando l'utente preme ENTER. Il parametro [s] è la
							riga di comando digitata dall'utente
						*/


private:
	static const u8		NUM_MAX_COLS = 200;
	static const u8		CURSOR_BLINK_TIME = 10;
	static const u16	HEADER_STARTY = 0;
	static const u16	HEADER_HEIGHT = 1;
	static const u16	BODY_STARTY = HEADER_STARTY + HEADER_HEIGHT;
	static const u16    MAX_INDENT_CHAR = 31;
	static const u16    INTERNAL_BUFFER_SIZE = 1024;

private:
	void                priv_buildIndentStr();
	void                priv_out(const char *what, u16 colorAttribute);
	void				priv_onKeyEvent(const KEY_EVENT_RECORD *ev);
	void				priv_onResizeEvent(const WINDOW_BUFFER_SIZE_RECORD *ev);
	void				priv_render();
	void				priv_clearInputBuffer();
	void				priv_appendStringToBody(const char *prefix, const char *s, WORD textAndBgAttribute);
	u16					priv_getBodyH() const																			{ return screenRows - BODY_STARTY - 1;  }
	void				priv_outTextCurrentBuffer(u16 colorAttribute);

private:
	OSCriticalSection	cs;
	HANDLE			hStdIN, hstdOUT, hConsoleBuffer1, hConsoleBuffer2, hCurConsoleBuffer;
	VideoBuffer		videoBufferForInput;
	VideoBuffer		videoBufferForOutput;
	VideoBuffer		videoBufferForHeader;
	u16				cursorX, cursorY, bodyY, bodyFirstVisibleRow;
	u8				timeToBlinkCursor;
	u8				cursorIsFull;
	u16				screenCols, screenRows;
	u16				inputLineNumber;
	bool			bQuitLoop;

private:
	u16             indent;
	char            strIndent[MAX_INDENT_CHAR + 1];
	char            buffer[INTERNAL_BUFFER_SIZE];
	u8              isANewLine;
};




#endif // _winTerminal_h_
