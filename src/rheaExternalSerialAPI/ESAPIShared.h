#ifndef _ESAPIShared_h_
#define _ESAPIShared_h_
#include "ESAPISubscriberList.h"

namespace esapi
{
	struct sShared
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

} // namespace esapi

#endif // _ESAPIShared_h_
