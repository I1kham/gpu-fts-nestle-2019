#include "CPUBridge.h"
#include "CPUBridgeServer.h"
#include "../rheaCommonLib/rheaUtils.h"

struct sThreadInitParam
{
	rhea::ISimpleLogger *logger;
	cpubridge::CPUChannel *chToCPU;
	HThreadMsgR chToThreadR;
	HThreadMsgW chToThreadW;
	OSEvent	hEvThreadStarted;
};


i16     cpuCommThreadFn (void *userParam);


//****************************************************************************
bool cpubridge::startServer (CPUChannel *chToCPU, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread, HThreadMsgW *out_hServiceChannelW)
{
	sThreadInitParam    init;

	//creo una coda FIFO da associare al thread in modo che sia possibile comunicare con il thread stesso
	rhea::thread::createMsgQ (&init.chToThreadR, &init.chToThreadW);


	//crea il thread
	init.logger = logger;
	init.chToCPU = chToCPU;
	rhea::event::open (&init.hEvThreadStarted);
	rhea::thread::create (out_hThread, cpuCommThreadFn, &init);

	//attendo che il thread del server sia partito
	bool bStarted = rhea::event::wait (init.hEvThreadStarted, 1000);
	rhea::event::close(init.hEvThreadStarted);

	if (bStarted)
	{
		*out_hServiceChannelW = init.chToThreadW;
		return true;
	}

	return false;
}


//*****************************************************************
i16 cpuCommThreadFn (void *userParam)
{
	sThreadInitParam *init = (sThreadInitParam*)userParam;
	HThreadMsgR chToThreadR = init->chToThreadR;
	HThreadMsgW chToThreadW = init->chToThreadW;

	cpubridge::Server server;
	server.useLogger(init->logger);
	if (server.start (init->chToCPU, chToThreadR))
	{
		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		server.run();
	}
	server.close();

	rhea::thread::deleteMsgQ (chToThreadR, chToThreadW);
	return 1;
}


/***************************************************
 * ritorna 0 se out_buffer non è abbastanza grande da contenere il messaggio.
 * altrimenti ritorna il num di byte inseriti in out_buffer
 */
u8 cpubridge_buildMsg (cpubridge::eCPUCommand command, const u8 *optionalData, u16 sizeOfOptionaData, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	//calcolo della dimensione totale
	if (sizeOfOutBuffer < 4 + sizeOfOptionaData)
		return 0;

	u8 ct = 0;
	out_buffer[ct++] = '#';
	out_buffer[ct++] = (u8)command;
	out_buffer[ct++] = 0;	//length

	if (optionalData && sizeOfOptionaData)
	{
		memcpy(&out_buffer[ct], optionalData, sizeOfOptionaData);
		ct += sizeOfOptionaData;
	}

	out_buffer[ct] = rhea::utils::simpleChecksum8_calc(out_buffer, ct);
	ct++;

	out_buffer[2] = ct;	//length
	return ct;
}

//***************************************************
u8 cpubridge::buildMsg_checkStatus_B (u8 keyPressed, u8 langErrorCode, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[8];
	u8 ct = 0;

	optionalData[ct++] = keyPressed;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = 0;
	optionalData[ct++] = langErrorCode;

	return cpubridge_buildMsg (cpubridge::eCPUCommand_checkStatus_B, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_initialParam_C(u8 gpuVersionMajor, u8 gpuVersionMinor, u8 gpuVersionBuild, u8 *out_buffer, u8 sizeOfOutBuffer)
{
	u8 optionalData[4];
	u8 ct = 0;

	optionalData[ct++] = gpuVersionMajor;
	optionalData[ct++] = gpuVersionMinor;
	optionalData[ct++] = gpuVersionBuild;

	return cpubridge_buildMsg (cpubridge::eCPUCommand_initialParam_C, optionalData, ct, out_buffer, sizeOfOutBuffer);
}

//***************************************************
u8 cpubridge::buildMsg_restart_U(u8 *out_buffer, u8 sizeOfOutBuffer)
{
	return cpubridge_buildMsg (cpubridge::eCPUCommand_restart, NULL, 0, out_buffer, sizeOfOutBuffer);
}



//***************************************************
void cpubridge::subscribe(const HThreadMsgW &hCPUMsgQWrite, const HThreadMsgW &hOtherMsgQWrite)
{
	u32 param32 = hOtherMsgQWrite.asU32();
	rhea::thread::pushMsg (hCPUMsgQWrite, CPUBRIDGE_SERVICECH_SUBSCRIPTION_REQUEST, param32);
}

//***************************************************
void cpubridge::translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, sSubscriber *out, u8 *out_cpuBridgeVersion)
{
	assert(msg.what == CPUBRIDGE_SERVICECH_SUBSCRIPTION_ANSWER);
	memcpy(out, msg.buffer, sizeof(sSubscriber));
	*out_cpuBridgeVersion = (u8)msg.paramU32;
}

//***************************************************
void cpubridge::unsubscribe(const sSubscriber &sub)
{
	rhea::thread::pushMsg(sub.hFromCpuToOtherW, CPUBRIDGE_SERVICECH_UNSUBSCRIPTION_REQUEST, (u32)0);
}


//***************************************************
void cpubridge::notify_CPUBRIDGE_DYING (const sSubscriber &to)
{
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_DYING, (u32)0);
}

//***************************************************
void cpubridge::notify_CPU_STATE_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, cpubridge::eVMCState VMCstate, u8 VMCerrorCode, u8 VMCerrorType)
{
	logger->log("notify_CPU_STATE_CHANGED\n");

	u8 state[4] = { (u8)VMCstate , VMCerrorCode, VMCerrorType, 0 };
	rhea::thread::pushMsg (to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED, handlerID, &state, 3);
}

//***************************************************
void cpubridge::translateNotify_CPU_STATE_CHANGED(const rhea::thread::sMsg &msg, cpubridge::eVMCState *out_VMCstate, u8 *out_VMCerrorCode, u8 *out_VMCerrorType)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED);
	
	const u8 *state = (u8*)msg.buffer;
	*out_VMCstate = (cpubridge::eVMCState)state[0];
	*out_VMCerrorCode = state[1];
	*out_VMCerrorType = state[2];
}

//***************************************************
void cpubridge::notify_CPU_CREDIT_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const void *credit, u8 sizeOfCredit)
{
	logger->log("notify_CPU_CREDIT_CHANGED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_STATE_CHANGED, handlerID, credit, sizeOfCredit);
}

//***************************************************
void cpubridge::translateNotify_CPU_CREDIT_CHANGED(const rhea::thread::sMsg &msg, u8 *out_credit, u16 sizeOfOut)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_CREDIT_CHANGED);
	u32 len = msg.bufferSize;
	if (sizeOfOut <= len)
		len = sizeOfOut;
	memcpy(out_credit, msg.buffer, len);
}

//***************************************************
void cpubridge::notify_CPU_NEW_LCD_MESSAGE (const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPULCDMessage *msg)
{
	logger->log("notify_CPU_NEW_LCD_MESSAGE\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE, handlerID, msg, sizeof(sCPULCDMessage));
}

//***************************************************
void cpubridge::translateNotify_CPU_NEW_LCD_MESSAGE (const rhea::thread::sMsg &msg, sCPULCDMessage *out_msg)
{
	assert (msg.what == CPUBRIDGE_NOTIFY_CPU_NEW_LCD_MESSAGE);
	memcpy(out_msg, msg.buffer, msg.bufferSize);
}

//***************************************************
void cpubridge::notify_CPU_SEL_AVAIL_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUSelAvailability *selAvailability)
{
	logger->log("notify_CPU_SEL_AVAIL_CHANGED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED, handlerID, selAvailability->_flag, sizeof(selAvailability->_flag));
}

//***************************************************
void cpubridge::translateNotify_CPU_SEL_AVAIL_CHANGED(const rhea::thread::sMsg &msg, sCPUSelAvailability *out)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SEL_AVAIL_CHANGED);
	memcpy(out->_flag, msg.buffer, sizeof(out->_flag));
}


//***************************************************
void cpubridge::notify_CPU_SEL_PRICES_CHANGED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const u16 *prices, u32 sizeOfPricesArray)
{
	logger->log("notify_CPU_SEL_PRICES_CHANGED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED, handlerID, prices, sizeOfPricesArray);
}

//***************************************************
void cpubridge::translateNotify_CPU_SEL_PRICES_CHANGED(const rhea::thread::sMsg &msg, u16 *out_prices, u32 sizeOfPricesArray)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_SEL_PRICES_CHANGED);
	assert(sizeOfPricesArray >= msg.bufferSize);
	memcpy(out_prices, msg.buffer, msg.bufferSize);
}

//***************************************************
void cpubridge::notify_CPU_RUNNING_SEL_STATUS(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eRunningSelStatus status)
{
	logger->log("notify_CPU_RUNNING_SEL_STATUS [%d]\n", (u8)status);

	u32 paramU32 = status;
	paramU32 <<= 16;
	paramU32 |= handlerID;
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS, paramU32);
}

//***************************************************
void cpubridge::translateNotify_CPU_RUNNING_SEL_STATUS(const rhea::thread::sMsg &msg, eRunningSelStatus *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_RUNNING_SEL_STATUS);
	*out_s = (eRunningSelStatus)((msg.paramU32 & 0xFFFF0000) >> 16);
}

//***************************************************
void cpubridge::notify_CPU_FULLSTATE(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUStatus *s)
{
	logger->log("notify_CPU_FULLSTATE\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_FULLSTATE, handlerID, s, sizeof(sCPUStatus));
}

//***************************************************
void cpubridge::translateNotify_CPU_FULLSTATE(const rhea::thread::sMsg &msg, sCPUStatus *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_FULLSTATE);
	memcpy(out_s, msg.buffer, sizeof(sCPUStatus));
}

//***************************************************
void cpubridge::notify_CPU_INI_PARAM(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, const sCPUParamIniziali *s)
{
	logger->log("notify_CPU_INI_PARAM\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_CPU_INI_PARAM, handlerID, s, sizeof(sCPUParamIniziali));
}

//***************************************************
void cpubridge::translateNotify_CPU_INI_PARAM(const rhea::thread::sMsg &msg, sCPUParamIniziali *out_s)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_CPU_INI_PARAM);
	memcpy(out_s, msg.buffer, sizeof(sCPUParamIniziali));
}

//***************************************************
void cpubridge::notify_CPU_BTN_PROG_PRESSED(const sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_CPU_BTN_PROG_PRESSED\n");
	rhea::thread::pushMsg(to.hFromCpuToOtherW, CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED, handlerID);
}

//***************************************************
void cpubridge::translateNotify_CPU_BTN_PROG_PRESSED(const rhea::thread::sMsg &msg)
{
	assert(msg.what == CPUBRIDGE_NOTIFY_BTN_PROG_PRESSED);
}



//***************************************************
void cpubridge::ask_CPU_START_SELECTION (const sSubscriber &from, u8 selNumber)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION, (u32)selNumber);
}

//***************************************************
void cpubridge::translate_CPU_START_SELECTION (const rhea::thread::sMsg &msg, u8 *out_selNumber)
{
	assert(msg.what == CPUBRIDGE_SUBSCRIBER_ASK_CPU_START_SELECTION);
	*out_selNumber = (eStatoPreparazioneBevanda)msg.paramU32;
}

//***************************************************
void cpubridge::ask_CPU_STOP_SELECTION (const sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_STOP_SELECTION, (u32)0);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_RUNNING_SEL_STATUS(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_RUNNING_SEL_STATUS, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_FULLSTATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_FULLSTATE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_INI_PARAM(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_INI_PARAM, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_AVAIL(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_AVAIL, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_SEL_PRICES (const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_SEL_PRICES, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_LCD_MESSAGE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_LCD_MESSAGE, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_CURRENT_CREDIT(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_CURRENT_CREDIT, handlerID);
}

//***************************************************
void cpubridge::ask_CPU_QUERY_STATE(const sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromOtherToCpuW, CPUBRIDGE_SUBSCRIBER_ASK_CPU_QUERY_STATE, handlerID);
}
