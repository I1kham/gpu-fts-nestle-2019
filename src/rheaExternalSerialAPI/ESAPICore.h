#ifndef _ESAPICore_h_
#define _ESAPICore_h_
#include "../rheaCommonLib/SimpleLogger/ISimpleLogger.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"
#include "../CPUBridge/CPUBridge.h"
#include "ESAPIShared.h"

namespace esapi
{
	/**************************************************
	 *	Core
	 */
	class Core
	{
	public:
								Core();
								~Core()													{ }

		void					useLogger (rhea::ISimpleLogger *loggerIN);
		bool					open (const char *serialPort, const HThreadMsgW &hCPUServiceChannelW);
		void					run();

		HThreadMsgW				getServiceMsgQQ() const														{ return shared.serviceMsgQW; }

	private:
		void					priv_close();
		bool					priv_subscribeToCPUBridge(const HThreadMsgW &hCPUServiceChannelW);

	private:
		sShared					shared;
		rhea::NullLogger        nullLogger;
		bool					bIsSubscribedToCPUBridge;
	};


} // namespace esapi

#endif // _ESAPICore_h_
