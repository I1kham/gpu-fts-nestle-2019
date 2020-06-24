#ifndef _ESAPIModule_h_
#define _ESAPIModule_h_
#include "ESAPIEnumAndDefine.h"
#include "ESAPISubscriberList.h"
#include "ESAPI.h"

namespace esapi
{
	class Module
	{
	public:
		struct sGlob
		{
			rhea::Allocator		*localAllocator;
			rhea::ISimpleLogger *logger;
			OSSerialPort		com;
			HThreadMsgW			hCPUServiceChannelW;
			HThreadMsgR			serviceMsgQR;
			HThreadMsgW			serviceMsgQW;
			SubscriberList		subscriberList;
			sESAPIModule		moduleInfo;
		};

	public:
		virtual							~Module()													{ }

		virtual  eExternalModuleType	run() = 0;
		
	};


} // namespace esapi

#endif // _ESAPIModule_h_
