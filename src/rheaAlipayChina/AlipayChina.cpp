#include "AlipayChina.h"
#include "AlipayChinaCore.h"

using namespace rhea;

struct sAlipayChinaInitParam
{
	OSEvent				hThreadStarted;
	AlipayChina::Core	*core;
};

i16     AlipayChinaThreadFn (void *userParam);

//****************************************************
bool AlipayChina::startThread (const char *serverIP, u16 serverPort, const char *machineID, const char *cryptoKey, rhea::ISimpleLogger *logger, AlipayChina::Context *out_context)
{
	Allocator *allocator = rhea::getSysHeapAllocator();
	out_context->core = RHEANEW(allocator, AlipayChina::Core)();
	out_context->core->useLogger (logger);
	bool ret = out_context->core->setup (serverIP, serverPort, machineID, cryptoKey, &out_context->hMsgQW);
	if (!ret)
	{
		RHEADELETE(allocator, out_context->core);
		return false;
	}

	//crea il thread
	sAlipayChinaInitParam ini;
	rhea::event::open (&ini.hThreadStarted);
	ini.core = out_context->core;
	rhea::thread::create (&out_context->hTread, AlipayChinaThreadFn, &ini);
	ret = rhea::event::wait (ini.hThreadStarted, 5000);
	rhea::event::close (ini.hThreadStarted);

	if (ret)
		return true;

	RHEADELETE(allocator, out_context->core);
	return false;
}


//****************************************************
i16 AlipayChinaThreadFn (void *userParam)
{
	sAlipayChinaInitParam *ini = (sAlipayChinaInitParam*)userParam;
	AlipayChina::Core *core = ini->core;
	rhea::event::fire(ini->hThreadStarted);
	core->run();
	return 0;
}


//****************************************************
void AlipayChina::subscribe (AlipayChina::Context &ctx, HThreadMsgW &hNotifyHere)
{
	u32 param32 = hNotifyHere.asU32();
	rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_SUBSCRIPTION_ANSWER, param32);
}

//****************************************************
void AlipayChina::kill (Context &ctx)
{
	rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_DIE, (u32)0);
}

//****************************************************
void AlipayChina::ask_ONLINE_STATUS (Context &ctx)
{
	rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_ASK_ONLINE_STATUS, (u32)0);
}

//****************************************************
void AlipayChina::ask_startOrder (Context &ctx, const u8 *selectionName, u8 selectionNum, const char *selectionPrice)
{
	u16 ct = 0;
	u8 data[256];
	
	data[ct++] = selectionNum;

	const u8 lenOfSelName = rhea::string::utf8::lengthInBytes(selectionName);
	data[ct++] = lenOfSelName;
	memcpy (&data[ct], selectionName, lenOfSelName);
	ct += lenOfSelName;
	data[ct++] = 0x00;

	const u8 lenOfSelPrice = (u8)strlen(selectionPrice);
	memcpy (&data[ct], selectionPrice, lenOfSelPrice);
	ct += lenOfSelPrice;
	data[ct++] = 0x00;

	rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_ASK_START_ORDER, data, ct);
}

//****************************************************
void AlipayChina::ask_abortOrder (Context &ctx)
{
	rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_ASK_ABORT_ORDER, (u32)0);
}

//****************************************************
void AlipayChina::ask_endOrder (Context &ctx, bool bSelectionWasDelivered)
{
	if (bSelectionWasDelivered)
		rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_ASK_END_ORDER, 1);
	else
		rhea::thread::pushMsg (ctx.hMsgQW, ALIPAYCHINA_ASK_END_ORDER, 0);
}
