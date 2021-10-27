#ifdef WIN32
#ifndef _winos_h_
#define _winos_h_
#include "winOSInclude.h"
#include "../../rheaDateTime.h"

namespace rhea
{
	class Allocator;
}

namespace platform
{
	namespace win32
	{
		bool			utf8_towchar (const u8 *utf8_string, u32 nBytesToUse, wchar_t* out, u32 sizeInBytesOfOut);
		bool			wchar_to_utf8 (const wchar_t* wstring, u32 nBytesToUse, u8* out, u32 sizeInBytesOfOut);
	};

	bool            internal_init (void *platformSpecificData, const char *appName);
	void            internal_deinit();

	void*           alignedAlloc(size_t alignment, size_t size);
	void			alignedFree(void *p);

	const u8*		getAppPathNoSlash();
	const u8*		getPhysicalPathToWritableFolder();

	uint64_t        getTimeNowMSec();
	void            sleepMSec(size_t msec);

	void            getDateNow(u16 *out_year, u16 *out_month, u16 *out_day);
	void            getTimeNow(u8 *out_hour, u8 *out_min, u8 *out_sec);

	void            runShellCommandNoWait(const char* cmdIN);

	eThreadError    createThread(OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam);
	void            killThread (OSThread &handle);
	void            waitThreadEnd(OSThread &handle);

	inline void     event_setInvalid(OSEvent &ev)										{ ev.h = INVALID_HANDLE_VALUE; }
	inline bool		event_isInvalid(const OSEvent &ev)									{ return (ev.h == INVALID_HANDLE_VALUE);  }
	inline bool     event_open (OSEvent *out_ev)										{ out_ev->h = ::CreateEvent(NULL, false, false, NULL); return true; }
	inline void     event_close (OSEvent &ev)											{ ::CloseHandle(ev.h); ev.h = INVALID_HANDLE_VALUE; }
	inline bool		event_compare(const OSEvent &a, const OSEvent &b)					{ return (a.h == b.h); }
	inline void     event_fire (const OSEvent &ev)										{ ::SetEvent(ev.h); }
	inline bool     event_wait (const OSEvent &ev, size_t timeoutMSec)					{ if (WAIT_OBJECT_0 == ::WaitForSingleObject(ev.h, timeoutMSec)) return true; return false; }

	inline void     criticalSection_init(OSCriticalSection *cs)							{ ::InitializeCriticalSection(&cs->cs); }
	inline void     criticalSection_close(OSCriticalSection &cs)						{ ::DeleteCriticalSection(&cs.cs); }
	inline bool     criticalSection_enter(OSCriticalSection &cs)						{ ::EnterCriticalSection(&cs.cs); return true; }
	inline void     criticalSection_leave(OSCriticalSection &cs)						{ ::LeaveCriticalSection(&cs.cs); }
	inline bool     criticalSection_tryEnter(OSCriticalSection &cs)						{ return (TryEnterCriticalSection(&cs.cs) == TRUE); }

					//====================================== file system
	bool			FS_DirectoryCreate(const u8 *utf8_path);
	bool			FS_DirectoryDelete(const u8 *utf8_path);
	bool			FS_DirectoryExists(const u8 *utf8_path);

	FILE*			FS_fileOpenForReadBinary (const u8 *utf8_fullFileNameAndPath);
	FILE*			FS_fileOpenForWriteBinary (const u8 *utf8_fullFileNameAndPath);
	FILE*			FS_fileOpenForReadText (const u8 *utf8_fullFileNameAndPath);
	FILE*			FS_fileOpenForWriteText (const u8 *utf8_fullFileNameAndPath);
	FILE*			FS_fileOpenForAppendText (const u8 *utf8_fullFileNameAndPath);

    void            FS_fileClose (FILE *f);

	bool			FS_fileExists(const u8 *utf8_filename);
	bool			FS_fileDelete(const u8 *utf8_filename);
	bool			FS_fileRename(const u8 *utf8_path, const u8 *utf8_oldFilename, const u8 *utf8_newFilename);

	bool			FS_findFirst (OSFileFind *h, const u8 *utf8_path, const u8 *utf8_jolly);
	bool			FS_findNext (OSFileFind &h);
	bool			FS_findIsDirectory (const OSFileFind &ff);
	void			FS_findGetFileName (const OSFileFind &ff, u8 *out, u32 sizeofOut);
	const u8*		FS_findGetFileName(const OSFileFind &ff);
	void			FS_findGetCreationTime (const OSFileFind &ff, rhea::DateTime *out);
	void			FS_findGetLastTimeModified (const OSFileFind &ff, rhea::DateTime *out);
	void			FS_findClose(OSFileFind &ff);

	bool			FS_findFirstHardDrive(OSDriveEnumerator *h, rheaFindHardDriveResult *out);
	bool			FS_findNextHardDrive(OSDriveEnumerator &h, rheaFindHardDriveResult *out);
	void			FS_findCloseHardDrive(OSDriveEnumerator &h);

	bool			FS_getDestkopPath(u8* out_path, u32 sizeof_out_path);

					//====================================== networking
	sNetworkAdapterInfo* NET_getListOfAllNerworkAdpaterIPAndNetmask (rhea::Allocator *allocator, u32 *out_numFound);
	bool				NET_getMACAddress (char *out_macAddress, u32 sizeOfMacAddress);
}   //namespace platform

#include "winOSSocket.h"
#include "winOSSerialPort.h"

#endif //_winos_h_
#endif //WIN32
