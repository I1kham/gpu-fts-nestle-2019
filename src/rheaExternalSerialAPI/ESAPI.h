#ifndef _ESAPI_h_
#define _ESAPI_h_
#include "../rheaCommonLib/rheaFastArray.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"


namespace esapi
{
	bool        startThread (const char *comPort, const HThreadMsgW &hCPUServiceChannelW, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread);
				/*
					Ritorna false in caso di problemi.
					Se ritorna true, allora:
						[out_hThread]				è l'handle del thread che è stato creato
				*/

	bool		isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse);

} // namespace esapi

#endif // _ESAPI_h_
