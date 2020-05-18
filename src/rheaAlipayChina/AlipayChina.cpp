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
	Allocator *allocator = rhea::memory_getDefaultAllocator();
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
	rhea::event::fire(ini->hThreadStarted);
	ini->core->run();
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