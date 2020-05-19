#ifdef LINUX
#ifndef _linuxos_h_
#define _linuxos_h_
#include "linuxOSInclude.h"
#include "../../rheaDateTime.h"


namespace platform
{
    bool            internal_init (void *platformSpecificData, const char *appName);
    void            internal_deinit ();

    void*           alignedAlloc (size_t alignment, size_t size);
    void            alignedFree (void *p);

    const char*     getAppPathNoSlash ();
    const char*     getPhysicalPathToWritableFolder();

    uint64_t        getTimeNowMSec();
    void            sleepMSec (size_t msec);

    void            getDateNow (u16 *out_year, u16 *out_month, u16 *out_day);
    void            getTimeNow (u8 *out_hour, u8 *out_min, u8 *out_sec);

    void            runShellCommandNoWait(const char* cmdIN);

    eThreadError    createThread (OSThread &out_handle, OSThreadFunction threadFunction, size_t stackSizeInKb, void *userParam);
    void            killThread (OSThread &handle);
    void            waitThreadEnd(OSThread &handle);

	inline void     event_setInvalid(OSEvent &ev)													{ ev.evfd = -1; }
	inline bool		event_isInvalid(const OSEvent &ev)												{ return (ev.evfd == -1); }
    bool            event_open (OSEvent *out_ev);
	inline bool		event_compare (const OSEvent &a, const OSEvent &b)								{ return (a.evfd == b.evfd); }
    void            event_close (OSEvent &ev);
    void            event_fire (const OSEvent &ev);
    bool            event_wait (const OSEvent &ev, size_t timeoutMSec);

    inline void     criticalSection_init (OSCriticalSection *cs)                                    { pthread_mutex_init (&cs->cs, NULL); }
    inline void     criticalSection_close (OSCriticalSection &cs)                                   { pthread_mutex_destroy(&cs.cs); }
    inline bool     criticalSection_enter (OSCriticalSection &cs)                                   { return (pthread_mutex_lock(&cs.cs) == 0); }
    inline void     criticalSection_leave (OSCriticalSection &cs)                                   { pthread_mutex_unlock(&cs.cs); }
    inline bool     criticalSection_tryEnter (OSCriticalSection &cs)                                { return (pthread_mutex_trylock(&cs.cs) == 0); }

                    //====================================== file system
    bool			FS_DirectoryCreate(const u8* const utf8_path);
    bool			FS_DirectoryDelete(const u8* const utf8_path);
    bool			FS_DirectoryExists(const     bool			FS_DirectoryDelete(const u8* const utf8_path);
utf8_path);
    
    FILE*			FS_fileOpenForReadBinary (const u8* const utf8_fullFileNameAndPath);
    FILE*			FS_fileOpenForWriteBinary (const u8* const utf8_fullFileNameAndPath);

    bool			FS_fileExists(const u8* const utf8_filename);
    bool			FS_fileDelete(const u8* const utf8_filename);
    bool			FS_fileRename(const u8 *utf8_path, const u8* utf8_oldFilename, const u8 *utf8_newFilename);

    bool			FS_findFirst (OSFileFind *h, const u8* const utf8_path, const u8* const utf8_jolly);
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
}   //namespace platform

#include "linuxOSSocket.h"
#include "linuxOSSerialPort.h"


#endif //_linuxos_h_
#endif
