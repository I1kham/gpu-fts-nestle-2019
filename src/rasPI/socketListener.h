#ifndef _socketListener_h_
#define _socketListener_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/Protocol/IProtocolChannell.h"
#include "../rheaCommonLib/Protocol/IProtocol.h"

namespace rasPI
{
	namespace socketListener
	{
		bool        start (rhea::ISimpleLogger *logger, rhea::HThread *out_hThread);
					/*	crea il thread che monitora e gestisce la socket. */
	} //socketListener
} //namespace rasPI

#endif //_socketListener_h_