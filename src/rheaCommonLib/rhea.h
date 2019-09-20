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




} //namespace rhea


#endif // _rhea_h_

