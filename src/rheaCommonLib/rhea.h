#ifndef _rhea_h_
#define _rhea_h_
#include "OS/OS.h"
#include "rheaEnumAndDefine.h"
#include "rheaMemory.h"
#include "rheaLogger.h"
#include "rheaDateTime.h"


namespace rhea
{
    bool                init (const char *appName, void *platformSpecificInitData);
    void                deinit();

    extern Logger       *sysLogger;

    inline const char*  getPhysicalPathToAppFolder()													{ return OS_getAppPathNoSlash();}
						//ritorna il path assoluto dell'applicazione, senza slash finale
	
	inline const char*	getPhysicalPathToWritableFolder()												{ return OS_getPhysicalPathToWritableFolder(); }
						//ritorna il path di una cartella nella quale è sicuramente possibile scrivere
						//Sui sistemi windows per es, ritorna una cosa del tipo "C:\Users\NOME_UTENTE".
						//Sui sistemi linux, ritorna generalmente lo stesso path dell'applicazione

	inline u64			getTimeNowMSec()																{ return OS_getTimeNowMSec(); }
						//ritorna il numero di msec trascorsi dall'avvio dell'applicazione

	const char*			getAppName();
	const DateTime&		getDateTimeAppStarted();
	const Date&			getDateAppStarted();
	const Time24&		getTimeAppStarted();
						//ritornano data e ora di avvio dell'applicazione

	f32					random01();
							//ritorna un num compreso tra 0 e 1 inclusi
	u32					randomU32(u32 iMax);
							//ritorna un u32 compreso tra 0 e iMax incluso



    inline void         shell_runCommandNoWait(const char *cmd)											{ OS_runShellCommandNoWait(cmd); }
						//esegue un comando di shall senza attenderne la terminazione

	bool				isLittleEndian();
	inline bool			isBigEndian()																	{ return !rhea::isLittleEndian(); }


	//file system stuff
	namespace fs
	{
		void				sanitizePath(const char *path, char *out_sanitizedPath, u32 sizeOfOutSanitzed);
		void				sanitizePathInPlace (char *path);
								//sostituisce \\ con /   rimuove eventuali . e .. e doppi /

		void				filePath_GoBack(const char *pathSenzaSlashIN, char *out, u32 sizeofout);
								// esegue un ".." sul filePathIN ritornando il nuovo path

		void				extractFileExt(const char *filename, char *ext, u32 sizeofext);
		void				extractFileNameWithExt(const char *filename, char *out, u32 sizeofOut);
		void				extractFileNameWithoutExt(const char *filename, char *out, u32 sizeofOut);
		void				extractFilePathWithSlash(const char *filename, char *out, u32 sizeofOut);
		void				extractFilePathWithOutSlash(const char *filename, char *out, u32 sizeofOut);
        bool                doesFileNameMatchJolly (const char *strFilename, const char *strJolly);

		inline bool			findFirstHardDrive(OSDriveEnumerator *h, rheaFindHardDriveResult *out)				{ return OS_FS_findFirstHardDrive(h, out); }
		inline bool			findNextHardDrive(OSDriveEnumerator &h, rheaFindHardDriveResult *out)				{ return OS_FS_findNextHardDrive(h, out); }
		inline void			findCloseHardDrive(OSDriveEnumerator &h)											{ OS_FS_findCloseHardDrive(h); }


		inline bool			fileExists(const char *fullFileNameAndPath)											{ return OS_FS_fileExists(fullFileNameAndPath); }
		inline bool         fileDelete(const char *fullFileNameAndPath)											{ return OS_FS_fileDelete(fullFileNameAndPath); }
		inline bool			fileRename(const char *oldFilename, const char *newFilename)						{ return OS_FS_fileRename(oldFilename, newFilename); }
		u64					filesize(FILE *fp);
		bool				fileCopy(const char *srcFullFileNameAndPath, const char *dstFullFileNameAndPath);
		u8*					fileCopyInMemory(const char *srcFullFileNameAndPath, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer);
		u8*					fileCopyInMemory(FILE *f, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer);

		inline bool			folderExists(const char *pathSenzaSlash)											{ return OS_FS_DirectoryExists(pathSenzaSlash); }
		inline bool			folderCreate(const char *pathSenzaSlash)											{ return OS_FS_DirectoryCreate(pathSenzaSlash); }
								//crea anche percorsi complessi. Es create("pippo/pluto/paperino), se necessario
								//prima crea pippo, poi pippo/pluto e infine pippo/pluto/paperino
		inline bool			folderDelete(const char *pathSenzaSlash)											{ return OS_FS_DirectoryDelete(pathSenzaSlash); }
		bool				folderCopy (const char *srcFullPathNoSlash, const char *dstFullPathNoSlash);
								//è ricorsiva, copia anche i sottofolder

		void				deleteAllFileInFolderRecursively(const char *pathSenzaSlash, bool bAlsoRemoveFolder);
							//cancella tutti i file del folder ed eventuali sottofolder

		inline bool			findFirst(OSFileFind *h, const char *strPathNoSlash, const char *strJolly)								{ return platform::FS_findFirst (h, strPathNoSlash, strJolly); }
		inline bool			findNext (OSFileFind &h)																				{ return platform::FS_findNext(h); }
		inline bool			findIsDirectory(const OSFileFind &h)																	{ return platform::FS_findIsDirectory(h); }
		inline void			findGetFileName (const OSFileFind &h, char *out, u32 sizeofOut)											{ platform::FS_findGetFileName(h, out, sizeofOut); }
		inline const char*	findGetFileName (const OSFileFind &h)																	{ return platform::FS_findGetFileName(h); }
		void				findComposeFullFilePathAndName(const OSFileFind &h, const char *pathNoSlash, char *out, u32 sizeofOut);
		inline void			findGetCreationTime(const OSFileFind &h, rhea::DateTime *out)											{ platform::FS_findGetCreationTime(h, out); }
		inline void			findGetLastTimeModified(const OSFileFind &h, rhea::DateTime *out)										{ platform::FS_findGetLastTimeModified(h, out); }
		inline void			findClose(OSFileFind &h)																				{ platform::FS_findClose(h); }

	}

	//network address stuff
	namespace netaddr
	{
		//=============================== NETWORK ADDRESS
		void				setFromSockAddr (OSNetAddr &me, const sockaddr_in &addrIN);
		void				setFromAddr (OSNetAddr &me, const OSNetAddr &addrIN);
		void				setIPv4(OSNetAddr &me, const char *ip);
		void				setPort(OSNetAddr &me, int port);
		bool				compare(const OSNetAddr &a, const OSNetAddr &b);
		void				getIPv4(const OSNetAddr &me, char *out);
		int					getPort(const OSNetAddr &me);
		sockaddr*			getSockAddr(const OSNetAddr &me);
		int					getSockAddrLen(const OSNetAddr &me);
	}



} //namespace rhea


#endif // _rhea_h_

