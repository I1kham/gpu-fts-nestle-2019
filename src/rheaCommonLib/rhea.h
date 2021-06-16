#ifndef _rhea_h_
#define _rhea_h_
#include "OS/OS.h"
#include "rheaEnumAndDefine.h"
#include "rheaMemory.h"
#include "rheaLogger.h"
#include "rheaDateTime.h"


namespace rhea
{
    bool                init (const char* appName, void *platformSpecificInitData);
    void                deinit();

    extern Logger       *sysLogger;

    inline const u8*	getPhysicalPathToAppFolder()													{ return platform::getAppPathNoSlash(); }
						//ritorna il path assoluto dell'applicazione, senza slash finale
	
	inline const u8*	getPhysicalPathToWritableFolder()												{ return platform::getPhysicalPathToWritableFolder(); }
						//ritorna il path di una cartella nella quale è sicuramente possibile scrivere
						//Sui sistemi windows per es, ritorna una cosa del tipo "C:\Users\NOME_UTENTE".
						//Sui sistemi linux, ritorna generalmente lo stesso path dell'applicazione

	inline u64			getTimeNowMSec()																{ return platform::getTimeNowMSec(); }
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

	inline Allocator*	getSysHeapAllocator()								{ return memory_getSysHeapAllocator(); }
	inline Allocator*	getScrapAllocator()									{ return memory_getScrapAllocator2(); }


    inline void         shell_runCommandNoWait(const char* cmd)										{ platform::runShellCommandNoWait(cmd); }
                        //esegue un comando di shell senza attenderne la terminazione

	bool				isLittleEndian();
	inline bool			isBigEndian()																	{ return !rhea::isLittleEndian(); }


	/************************************************************************************************************
	 *
	 * file system
	 *
	 */
	namespace fs
	{
		void				sanitizePathInPlace (u8 *utf8_path, u32 nBytesToCheck=u32MAX);
		void				sanitizePath (const u8* utf8_path, u8* out_utf8sanitizedPath, u32 sizeOfOutSanitzed);
								/* rimuove eventuali . e .. e doppi slash in favore del singolo slash alla unix (/) 
								*/

		void				filePath_GoBack (const u8* utf8_pathSenzaSlashIN, u8 *utf8_out, u32 sizeofout);
								// esegue un ".." sul filePathIN ritornando il nuovo path

		inline bool			getDestkopPath (u8* out_path, u32 sizeof_out_path)						{ return platform::FS_getDestkopPath(out_path, sizeof_out_path); }

		void				extractFileExt (const u8* utf8_filename, u8 *utf8_ext, u32 sizeofext);
		void				extractFileNameWithExt (const u8* utf8_filename, u8 *utf8_out, u32 sizeofOut);
		void				extractFileNameWithoutExt (const u8* utf8_filename, u8 *utf8_out, u32 sizeofOut);
		void				extractFilePathWithSlash (const u8* utf8_filename, u8 *utf8_out, u32 sizeofOut);
		void				extractFilePathWithOutSlash (const u8* utf8_filename, u8 *utf8_out, u32 sizeofOut);

        bool				doesFileNameMatchJolly (const u8* utf8_strFilename, const u8 *utf8_strJolly);
								//la stringa dei jolly può contenere più di una sequenza. Le sequenze sono separate da spazio (es: "*.txt *.bmp")

		inline bool			findFirstHardDrive(OSDriveEnumerator *h, rheaFindHardDriveResult *out)						{ return platform::FS_findFirstHardDrive(h, out); }
		inline bool			findNextHardDrive(OSDriveEnumerator &h, rheaFindHardDriveResult *out)						{ return platform::FS_findNextHardDrive(h, out); }
		inline void			findCloseHardDrive(OSDriveEnumerator &h)													{ platform::FS_findCloseHardDrive(h); }


        inline bool			fileExists(const u8* utf8_fullFileNameAndPath)                                              { return platform::FS_fileExists(utf8_fullFileNameAndPath); }
        inline bool         fileDelete(const u8* utf8_fullFileNameAndPath)                                              { return platform::FS_fileDelete(utf8_fullFileNameAndPath); }
		inline bool			fileRename(const u8 *utf8_path, const u8* utf8_oldFilename, const u8 *utf8_newFilename)		{ return platform::FS_fileRename(utf8_path, utf8_oldFilename, utf8_newFilename); }
		
        inline FILE*		fileOpenForReadBinary (const u8* utf8_fullFileNameAndPath)                                  { return platform::FS_fileOpenForReadBinary(utf8_fullFileNameAndPath); }
        inline FILE*		fileOpenForWriteBinary (const u8* utf8_fullFileNameAndPath)                                 { return platform::FS_fileOpenForWriteBinary(utf8_fullFileNameAndPath); }
        inline FILE*		fileOpenForReadText (const u8* utf8_fullFileNameAndPath)                                    { return platform::FS_fileOpenForReadText(utf8_fullFileNameAndPath); }
        inline FILE*		fileOpenForWriteText (const u8* utf8_fullFileNameAndPath)                                   { return platform::FS_fileOpenForWriteText(utf8_fullFileNameAndPath); }
        inline FILE*		fileOpenForAppendText (const u8* utf8_fullFileNameAndPath)                                  { return platform::FS_fileOpenForAppendText(utf8_fullFileNameAndPath); }

        inline void         fileClose (FILE *f)                                                                         { return platform::FS_fileClose(f); }

		u64					filesize(const u8* utf8_srcFullFileNameAndPath);
		u64					filesize(FILE *fp);
		bool				fileCopy(const u8* utf8_srcFullFileNameAndPath, const u8* utf8_dstFullFileNameAndPath);
		void				fileCopyInChunkWithPreallocatedBuffer (FILE *fSRC, u32 numBytesToCopy, FILE *fDST, void *buffer, u32 BUFFER_SIZE);
		u8*					fileCopyInMemory(const u8* srcFullFileNameAndPath, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer);
		u8*					fileCopyInMemory(FILE *f, rhea::Allocator *allocator, u32 *out_sizeOfAllocatedBuffer);
		u32					fileRead (FILE *f, u8 *out_buffer, u32 numBytesToRead);
		u32					fileWrite (FILE *f, const u8 *buffer, u32 numBytesToWrite);

		inline bool			folderExists(const u8* pathSenzaSlash)											{ return platform::FS_DirectoryExists(pathSenzaSlash); }
		inline bool			folderCreate(const u8* pathSenzaSlash)											{ return platform::FS_DirectoryCreate(pathSenzaSlash); }
								//crea anche percorsi complessi. Es create("pippo/pluto/paperino), se necessario
								//prima crea pippo, poi pippo/pluto e infine pippo/pluto/paperino
		inline bool			folderDelete(const u8* pathSenzaSlash)											{ return platform::FS_DirectoryDelete(pathSenzaSlash); }
		bool				folderCopy (const u8* utf8_srcFullPathNoSlash, const u8* utf8_dstFullPathNoSlash, const u8* *elencoPathDaEscludere=NULL);
								//è ricorsiva, copia anche i sottofolder

		void				deleteAllFileInFolderRecursively(const u8* pathSenzaSlash, bool bAlsoRemoveFolder);
							//cancella tutti i file del folder ed eventuali sottofolder

		inline bool			findFirst(OSFileFind *h, const u8* utf8_path, const u8* utf8_jolly)							{ return platform::FS_findFirst (h, utf8_path, utf8_jolly); }
		inline bool			findNext (OSFileFind &h)																				{ return platform::FS_findNext(h); }
		inline bool			findIsDirectory(const OSFileFind &h)																	{ return platform::FS_findIsDirectory(h); }
		inline void			findGetFileName (const OSFileFind &h, u8 *out, u32 sizeofOut)											{ platform::FS_findGetFileName(h, out, sizeofOut); }
		inline const u8*	findGetFileName (const OSFileFind &h)																	{ return platform::FS_findGetFileName(h); }
		void				findComposeFullFilePathAndName(const OSFileFind &h, const u8* pathNoSlash, u8 *out, u32 sizeofOut);
		inline void			findGetCreationTime(const OSFileFind &h, rhea::DateTime *out)											{ platform::FS_findGetCreationTime(h, out); }
		inline void			findGetLastTimeModified(const OSFileFind &h, rhea::DateTime *out)										{ platform::FS_findGetLastTimeModified(h, out); }
		inline void			findClose(OSFileFind &h)																				{ platform::FS_findClose(h); }
	} //namespace fs

	/************************************************************************************************************
	 *
	 * network & socket
	 *
	 */
	namespace netaddr
	{
		inline  sNetworkAdapterInfo*	getListOfAllNerworkAdpaterIPAndNetmask(rhea::Allocator *allocator, u32 *out_numFound)		{ return platform::NET_getListOfAllNerworkAdpaterIPAndNetmask(allocator, out_numFound); }
									//alloca un array di [sNetworkAdapterInfo] e lo ritorna.
									//L'array contiene le info su nome, IP e netmask degli adattaotri di rete disponibili al sistema
		void				ipstrTo4bytes (const char *ip, u8 *out_b1, u8 *out_b2, u8 *out_b3, u8 *out_b4);

		//=============================== NETWORK ADDRESS
		void				setFromSockAddr(OSNetAddr &me, const sockaddr_in &addrIN);
		void				setFromAddr(OSNetAddr &me, const OSNetAddr &addrIN);
		void				setIPv4(OSNetAddr &me, const char*ip);
		void				setPort(OSNetAddr &me, int port);
		bool				compare(const OSNetAddr &a, const OSNetAddr &b);
		void				getIPv4(const OSNetAddr &me, char *out);
		int					getPort(const OSNetAddr &me);
		sockaddr*			getSockAddr(const OSNetAddr &me);
		int					getSockAddrLen(const OSNetAddr &me);
	} //namespace netaddr

	namespace socket
	{
		inline void					init (OSSocket *sok)																	{ platform::socket_init(sok); }

		//=============================================== TCP
		inline eSocketError         openAsTCPServer(OSSocket *out_sok, int portNumber)										{ return platform::socket_openAsTCPServer(out_sok, portNumber); }
		inline eSocketError         openAsTCPClient(OSSocket *out_sok, const char* connectToIP, u32 portNumber)				{ return platform::socket_openAsTCPClient(out_sok, connectToIP, portNumber); }

		inline void                 close(OSSocket &sok)																	{ platform::socket_close(sok); }

		inline bool                 isOpen(const OSSocket &sok)																{ return platform::socket_isOpen(sok); }
			/* false se la socket non è open.
			 * False anche a seguito di una chiamata a close() (in quanto la sok viene chiusa)
			 */

		inline bool                 compare(const OSSocket &a, const OSSocket &b)											{ return platform::socket_compare(a, b); }
			/* true se "puntano" alla stessa socket
			*/


		inline bool                 setReadTimeoutMSec(OSSocket &sok, u32 timeoutMSec)										{ return platform::socket_setReadTimeoutMSec(sok, timeoutMSec); }
			/* Per specificare un tempo di wait "infinito" (ie: socket sempre bloccante), usare timeoutMSec=u32MAX
			 * Per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
			 * Tutti gli altri valori sono comunque validi ma non assumono significati particolari
			 */

		inline bool                 setWriteTimeoutMSec(OSSocket &sok, u32 timeoutMSec)										{ return platform::socket_setWriteTimeoutMSec(sok, timeoutMSec); }
			/* Per specificare un tempo di wait "infinito" (ie: socket sempre bloccante), usare timeoutMSec=u32MAX
			 * Per indicare il tempo di wait minimo possibile, usare timeoutMSec=0
			 * Tutti gli altri valori sono comunque validi ma non assumono significati particolari
			 */


		inline bool					listen(const OSSocket &sok, u16 maxIncomingConnectionQueueLength = u16MAX)					{ return platform::socket_listen(sok, maxIncomingConnectionQueueLength); }
		inline bool					accept(const OSSocket &sok, OSSocket *out_clientSocket)										{ return platform::socket_accept(sok, out_clientSocket); }

		inline i32					read(OSSocket &sok, void *buffer, u16 bufferSizeInBytes, u32 timeoutMSec, bool bPeekMsg=false)	{ return platform::socket_read(sok, buffer, bufferSizeInBytes, timeoutMSec, bPeekMsg); }
			/* prova a leggere dalla socket. La chiamata è bloccante per un massimo di timeoutMSec.
			 * Riguardo [timeoutMSec], valgono le stesse considerazioni indicate in setReadTimeoutMSec()
			 *
			 * Ritorna:
			 *      0   se la socket si è disconnessa
			 *      -1  se la chiamata avrebbe bloccato il processo (quindi devi ripetere la chiamata fra un po')
			 *      >0  se ha letto qualcosa e ha quindi fillato [buffer] con il num di bytes ritornato
			 */

		inline i32                  write(OSSocket &sok, const void *buffer, u16 nBytesToSend)								{ return platform::socket_write(sok, buffer, nBytesToSend); }
			/*	Ritorna il numero di btye scritti sulla socket.
			 *	Se ritorna 0, vuol dire che la chiamata sarebbe stata bloccante e quindi
			 *	l'ha evitata
			 */

		 //=============================================== UDP
		inline	eSocketError		openAsUDP(OSSocket *out_sok)															{ return platform::socket_openAsUDP(out_sok); }
		inline	eSocketError		UDPbind(OSSocket &sok, int portNumber)													{ return platform::socket_UDPbind(sok, portNumber); }
		inline	u32					UDPSendTo(OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const OSNetAddr &addrTo)	{ return platform::socket_UDPSendTo(sok, buffer, nBytesToSend, addrTo); }
		inline	u32					UDPReceiveFrom(OSSocket &sok, u8 *buffer, u32 nMaxBytesToRead, OSNetAddr *out_from)		{ return platform::socket_UDPReceiveFrom(sok, buffer, nMaxBytesToRead, out_from); }
		void						UDPSendBroadcast (OSSocket &sok, const u8 *buffer, u32 nBytesToSend, const char *ip, int portNumber, const char *constsubnetMask);
	}
	

	/************************************************************************************************************
	 *
	 * rsr232
	 *
	 */
	namespace rs232
	{
		inline void     setInvalid(OSSerialPort &sp)													{ return platform::serialPort_setInvalid(sp); }
		inline bool     isInvalid(const OSSerialPort &sp)												{ return platform::serialPort_isInvalid(sp); }
		inline bool     isValid(const OSSerialPort &sp)													{ return !platform::serialPort_isInvalid(sp); }

		inline bool     open(OSSerialPort *out_serialPort, const char *deviceName,
			eRS232BaudRate baudRate,
			bool RST_on,
			bool DTR_on,
            eRS232DataBits dataBits = eRS232DataBits_8,
            eRS232Parity parity = eRS232Parity_No,
            eRS232StopBits stop = eRS232StopBits_One,
            eRS232FlowControl flowCtrl = eRS232FlowControl_No,
			bool bBlocking = true)																		{ return platform::serialPort_open(out_serialPort, deviceName, baudRate, RST_on, DTR_on, dataBits, parity, stop, flowCtrl, bBlocking); }

		inline void     close(OSSerialPort &sp)															{ return platform::serialPort_close(sp); }

		inline void     setRTS(OSSerialPort &sp, bool bON_OFF)											{ platform::serialPort_setRTS(sp, bON_OFF); }
		inline void     setDTR(OSSerialPort &sp, bool bON_OFF)											{ platform::serialPort_setDTR(sp, bON_OFF); }

		inline void     flushIO(OSSerialPort &sp)														{ platform::serialPort_flushIO(sp); }
		/* flusha i buffer di input e output discardando tutto quanto
		 */

		inline u32      readBuffer(OSSerialPort &sp, void *out_byteRead, u32 numMaxByteToRead)			{ return platform::serialPort_readBuffer(sp, out_byteRead, numMaxByteToRead); }
		/* legge al massimo [numMaxByteToRead] bytes dalla seriale e li memorizza in [out_byteRead]
		 * Ritorna il numero di byte letti e memorizzati in [out_byteRead]
		 *
		 * Se la seriale è in modalità bloccante, questa fn è a sua volta bloccante
		 */

		inline bool     readByte(OSSerialPort &sp, u8 *out_b)											{ return (platform::serialPort_readBuffer(sp, out_b, 1) == 1); }
		/* true se ha letto un btye dalla seriale nel qual caso [out_b] contiene il byte letto
		 * Valgono le stesse indicazioni valide per readBuffer()
		 */


		inline u32      writeBuffer(OSSerialPort &sp, const void *buffer, u32 nBytesToWrite)			{ return platform::serialPort_writeBuffer(sp, buffer, nBytesToWrite); }
		/* prova a scrivere fino a [nBytesToWrite].
		 * Ritorna il numero di bytes scritti con successo
		 *
		 * Ritorna 0 o <0 in caso di errore
		 */

		inline bool     writeByte(OSSerialPort &sp, u8 byteToWrite)										{ return (platform::serialPort_writeBuffer(sp, &byteToWrite, 1) == 1); }
	} //namespace rs232



} //namespace rhea


#endif // _rhea_h_

