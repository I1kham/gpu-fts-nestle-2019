#ifndef _AlipayChinaEnumAndDefine_h_
#define _AlipayChinaEnumAndDefine_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"

#define ALIPAYCHINA_SUBSCRIPTION_ANSWER					0x1100
#define ALIPAYCHINA_SUBSCRIPTION_ANSWER_ACCEPTED		0x1101
#define ALIPAYCHINA_DIE									0x1102

#define ALIPAYCHINA_ASK_ONLINE_STATUS					0x1300
#define ALIPAYCHINA_NOTIFY_ONLINE_STATUS_CHANGED		0x1200

namespace rhea
{
    namespace AlipayChina
    {
		class Core;

		struct Context
		{
            HThread			hTread;
            HThreadMsgW		hMsgQW;
			Core			*core;
        };
    } //namespace AlipayChina
} // namespace rhea

#endif // _AlipayChinaEnumAndDefine_h_
