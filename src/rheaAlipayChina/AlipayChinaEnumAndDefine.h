#ifndef _AlipayChinaEnumAndDefine_h_
#define _AlipayChinaEnumAndDefine_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaThread.h"
#include "../rheaCommonLib/SimpleLogger/NullLogger.h"

#define ALIPAYCHINA_SUBSCRIPTION_ANSWER					0x1100
#define ALIPAYCHINA_SUBSCRIPTION_ANSWER_ACCEPTED		0x1101
#define ALIPAYCHINA_DIE									0x1102

#define ALIPAYCHINA_ASK_ONLINE_STATUS					0x1300
#define ALIPAYCHINA_ASK_START_ORDER                     0x1301
#define ALIPAYCHINA_ASK_END_ORDER                       0x1302
#define ALIPAYCHINA_ASK_ABORT_ORDER                     0x1303

#define ALIPAYCHINA_NOTIFY_ONLINE_STATUS_CHANGED		0x1200
#define ALIPAYCHINA_NOTIFY_ORDER_STATUS     		    0x1201

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

        enum class eOrderStatus: u8
        {
            waitingQR = 0,
            pollingPaymentStatus = 1,
            paymentOK = 2,
            paymentTimeout = 3
        };
    } //namespace AlipayChina
} // namespace rhea

#endif // _AlipayChinaEnumAndDefine_h_
