#include "ESAPI.h"
#include "ESAPICore.h"
#include "../rheaCommonLib/rheaUtils.h"


struct sEsapiThreadInitParam
{
	rhea::ISimpleLogger *logger;
	char				comPort[64];
	HThreadMsgW			hCPUServiceChannelW;
	OSEvent				hEvThreadStarted;
};
HThreadMsgW esapi_serviceMsgQW;

i16     esapiThreadFn (void *userParam);

//****************************************************************************
bool esapi::startThread (const char *comPort, const HThreadMsgW &hCPUServiceChannelW, rhea::ISimpleLogger *logger, rhea::HThread *out_hThread)
{
	sEsapiThreadInitParam    init;

	//crea il thread
	init.logger = logger;
	init.hCPUServiceChannelW = hCPUServiceChannelW;
	sprintf_s (init.comPort, sizeof(init.comPort), "%s", comPort);
	rhea::event::open (&init.hEvThreadStarted);
	rhea::thread::create (out_hThread, esapiThreadFn, &init);

	//attendo che il thread sia partito
	bool bStarted = rhea::event::wait (init.hEvThreadStarted, 1000);
	rhea::event::close(init.hEvThreadStarted);

	if (bStarted)
	{
		return true;
	}

	return false;
}

//*****************************************************************
i16 esapiThreadFn (void *userParam)
{
	sEsapiThreadInitParam *init = (sEsapiThreadInitParam*)userParam;

	esapi::Core core;
	core.useLogger(init->logger);
	if (core.open (init->comPort, init->hCPUServiceChannelW))
	{
		esapi_serviceMsgQW = core.getServiceMsgQQ();

		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		core.run();
	}

	return 1;
}

//*****************************************************************
void esapi::subscribe (const HThreadMsgW &hAnswerHere)
{
	u32 param32 = hAnswerHere.asU32();
	rhea::thread::pushMsg (esapi_serviceMsgQW, ESAPI_SERVICECH_SUBSCRIPTION_REQUEST, param32);
}

//*****************************************************************
void esapi::translate_SUBSCRIPTION_ANSWER (const rhea::thread::sMsg &msg, cpubridge::sSubscriber *out)
{
    assert(msg.what == ESAPI_SERVICECH_SUBSCRIPTION_ANSWER);
	memcpy(out, msg.buffer, sizeof(cpubridge::sSubscriber));
}

//*****************************************************************
void esapi::unsubscribe (const cpubridge::sSubscriber &sub)
{
	ask_UNSUBSCRIBE(sub);
}


//****************************************************************************
bool esapi::isValidChecksum (u8 ck, const u8 *buffer, u32 numBytesToUse)
{
	return (ck == rhea::utils::simpleChecksum8_calc(buffer, numBytesToUse));
}


//*********************************************************
u32 priv_esapi_buildMsg (u8 c1, u8 c2, const u8* optionalData, u32 numOfBytesInOptionalData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 4 + numOfBytesInOptionalData;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = c1;
	out_buffer[ct++] = c2;
    if (NULL != optionalData && numOfBytesInOptionalData)
    {
        memcpy (&out_buffer[ct], optionalData, numOfBytesInOptionalData);
        ct += numOfBytesInOptionalData;
    }

    out_buffer[ct] = rhea::utils::simpleChecksum8_calc (out_buffer, ct);
    ct++;

	return ct;
}

//****************************************************************************
u32 esapi::buildMsg_A1_getAPIVersion_ask (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('A', '1', NULL, 0, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_A1_getAPIVersion_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk)
{
	//# A 1 [ck]
	const u32 MSG_LEN = 4;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
	}

	return MSG_LEN;
}

u32 esapi::buildMsg_A1_getAPIVersion_resp (u8 apiVerMajor, u8 apiVerMinor, eGPUType gpuType, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u8 data[4] = { apiVerMajor, apiVerMinor, (u8)gpuType, 0 };
	return priv_esapi_buildMsg ('A', '1', data, 3, out_buffer, sizeOfOutBuffer);
}
u32	esapi::buildMsg_A1_getAPIVersion_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_apiVerMajor, u8 *out_apiVerMinor, eGPUType *out_gpuType)
{
	//# A 1 [api_ver_major] [api_ver_minor] [GPUmodel] [ck]
	const u32 MSG_LEN = 7;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_apiVerMajor = buffer[3];
		*out_apiVerMinor = buffer[4];
		*out_gpuType = (eGPUType)buffer[5];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_apiVerMajor = 0;
		*out_apiVerMinor = 0;
		*out_gpuType = esapi::eGPUType_unknown;
	}
	return MSG_LEN;
}

//****************************************************************************
u32 esapi::buildMsg_C1_getCPUScreenMsg_ask (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('C', '1', NULL, 0, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_C1_getCPUScreenMsg_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk)
{
	//# C 1 [ck]
	const u32 MSG_LEN = 4;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
	}

	return MSG_LEN;
}

u32 esapi::buildMsg_C1_getCPUScreenMsg_resp (const void *lcdMsg, u16 numBytesInMsg, u8 *out_buffer, u32 sizeOfOutBuffer)
{
    //# C 1 [numBytesInMsg] [msgUTF16_LSB_MSB] [ck]
    const u32 totalSizeOfMsg = 5 + numBytesInMsg;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    assert (numBytesInMsg <= 256);

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'C';
    out_buffer[ct++] = '1';
    out_buffer[ct++] = (u8)(numBytesInMsg);
	
    if (numBytesInMsg > 0)
    {
        memcpy(&out_buffer[ct], lcdMsg, numBytesInMsg);
        ct += numBytesInMsg;
    }

    out_buffer[ct] = rhea::utils::simpleChecksum8_calc (out_buffer, ct);
	return ct+1;
}

//****************************************************************************
u32 esapi::buildMsg_C2_getSelAvailability_ask (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('C', '2', NULL, 0, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_C2_getSelAvailability_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk)
{
	//# C 2 [ck]
	const u32 MSG_LEN = 4;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
	}

	return MSG_LEN;
}
u32 esapi::buildMsg_C2_getSelAvailability_resp (const cpubridge::sCPUSelAvailability &selAvail, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 20;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'C';
	out_buffer[ct++] = '2';
	
	memset (&out_buffer[3], 0x00, 16);
	for (u8 i = 1; i <= NUM_MAX_SELECTIONS; i++)
	{
        if (selAvail.isAvail(i))
            rhea::bit::set (&out_buffer[3], 16, i-1);
	}
    out_buffer[19] = rhea::utils::simpleChecksum8_calc (out_buffer, 19);
	return 20;
}

//****************************************************************************
u32	esapi::buildMsg_C3_getSelAvailability_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk)
{
	//# C 3 [ck]
	const u32 MSG_LEN = 4;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
	}

	return MSG_LEN;
}

//****************************************************************************
u32 esapi::buildMsg_S1_startSelection_ask (u8 selNum, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('S', '1', &selNum, 1, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_S1_startSelection_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_selNum)
{
	//# S 1 [sel_num] [ck]
	const u32 MSG_LEN = 5;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_selNum = buffer[3];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_selNum = 0;
	}
	return MSG_LEN;
}

u32 esapi::buildMsg_S1_startSelection_resp (u8 selNum, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('S', '1', &selNum, 1, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_S1_startSelection_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_selNum)
{
	//# S 1 [sel_num] [ck]
	const u32 MSG_LEN = 5;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_selNum = buffer[3];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_selNum = 0;
	}
	return MSG_LEN;
}


//****************************************************************************
u32 esapi::buildMsg_S2_querySelectionStatus_ask (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('S', '2', NULL, 0, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_S2_querySelectionStatus_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk)
{
	//# S 2 [ck]
	const u32 MSG_LEN = 4;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
	}

	return MSG_LEN;
}

u32 esapi::buildMsg_S2_querySelectionStatus_resp (cpubridge::eRunningSelStatus status, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u8 data = (u8)status;
	return priv_esapi_buildMsg ('S', '2', &data, 1, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_S2_querySelectionStatus_parseResp (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, cpubridge::eRunningSelStatus *out_sstatus)
{
	//# S 2 [status] [ck]
	const u32 MSG_LEN = 5;
	if (numBytesInBuffer < MSG_LEN)
		return 0;
	
	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_sstatus = (cpubridge::eRunningSelStatus)buffer[3];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_sstatus = cpubridge::eRunningSelStatus_wait;
	}

	return MSG_LEN;
}


//****************************************************************************
u32	esapi::buildMsg_S3_startAlreadySelection_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_selNum, u16 *out_price)
{
	//# S 3 [sel_num] [priceLSB] [priceMSB] [ck]
	const u32 MSG_LEN = 7;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_selNum = buffer[3];
		*out_price = rhea::utils::bufferReadU16_LSB_MSB(&buffer[4]);
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_selNum = 0;
		*out_price = 0;
	}
	return MSG_LEN;
}
u32	esapi::buildMsg_S3_startAlreadySelection_resp (u8 selNum, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('S', '3', &selNum, 1, out_buffer, sizeOfOutBuffer);
}


//****************************************************************************
u32 esapi::buildMsg_S4_btnPress_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u8 *out_btnNum)
{
	//# S 4 [btn_num] [ck]
	const u32 MSG_LEN = 5;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_btnNum = buffer[3];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_btnNum = 0;
	}
	return MSG_LEN;
}
u32 esapi::buildMsg_S4_btnPress_resp (u8 btnNum, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('S', '4', &btnNum, 1, out_buffer, sizeOfOutBuffer);
}


//****************************************************************************
u32 esapi::buildMsg_R1_externalModuleIdentify_ask (esapi::eExternalModuleType moduleType, u8 verMajor, u8 verMinor, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	u8 data[4] = { (u8)moduleType, verMajor, verMinor, 0 };
	return priv_esapi_buildMsg ('R', '1', data, 3, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_R1_externalModuleIdentify_parseAsk (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, eExternalModuleType *out_moduleType, u8 *out_verMajor, u8 *out_verMinor)
{
	//# R 1 [moduleType] [verMajor] [verMinor] [ck]
	const u32 MSG_LEN = 7;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_moduleType = (eExternalModuleType)buffer[3];
		*out_verMajor = buffer[4];
		*out_verMinor = buffer[5];
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
        *out_moduleType = eExternalModuleType_none;
		*out_verMajor = 0;
		*out_verMinor = 0;
	}

	return MSG_LEN;
}
u32 esapi::buildMsg_R1_externalModuleIdentify_resp (u8 result, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('R', '1', &result, 1, out_buffer, sizeOfOutBuffer);
}

//****************************************************************************
u32 esapi::buildMsg_R0x01_newSocket (u32 socketUID, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	u8 data[4];
	rhea::utils::bufferWriteU32 (data, socketUID);
	return priv_esapi_buildMsg ('R', 0x01, data, 4, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_R0x01_newSocket_parse (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u32 *out_socketUID)
{
	//# R [0x01] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
	const u32 MSG_LEN = 8;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_socketUID = rhea::utils::bufferReadU32(&buffer[3]);
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_socketUID = 0;
	}

	return MSG_LEN;
}

//****************************************************************************
u32 esapi::buildMsg_R0x02_closeSocket (u32 socketUID, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	u8 data[4];
	rhea::utils::bufferWriteU32 (data, socketUID);
	return priv_esapi_buildMsg ('R', 0x02, data, 4, out_buffer, sizeOfOutBuffer);
}
u32 esapi::buildMsg_R0x02_closeSocket_parse (const u8 *buffer, u32 numBytesInBuffer, bool *out_bIsValidCk, u32 *out_socketUID)
{
	//# R [0x02] [socketUID_MSB3] [socketUID_MSB2] [socketUID_MSB1] [socketUID_LSB] [ck]
	const u32 MSG_LEN = 8;
	if (numBytesInBuffer < MSG_LEN)
		return 0;

	if (buffer[MSG_LEN - 1] == rhea::utils::simpleChecksum8_calc(buffer, MSG_LEN - 1))
	{
		*out_bIsValidCk = true;
		*out_socketUID = rhea::utils::bufferReadU32(&buffer[3]);
	}
	else
	{
		DBGBREAK;
		*out_bIsValidCk = false;
		*out_socketUID = 0;
	}

	return MSG_LEN;
}

//****************************************************************************
u32 esapi::buildMsg_R0x03_socketDataToGPU (u32 socketUID, const u8 *data, u16 lenOfData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 10 + lenOfData;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'R';
	out_buffer[ct++] = 0x03;
	
	rhea::utils::bufferWriteU32 (&out_buffer[ct], socketUID);
	ct += 4;

	rhea::utils::bufferWriteU16 (&out_buffer[ct], lenOfData);
	ct += 2;

	if (NULL != data && lenOfData)
    {
        memcpy (&out_buffer[ct], data, lenOfData);
        ct += lenOfData;
    }

    out_buffer[ct] = rhea::utils::simpleChecksum8_calc (out_buffer, ct);
    ct++;

	return ct;
}

//****************************************************************************
u32 esapi::buildMsg_R0x04_GPUDataToSocket (u32 socketUID, const u8 *data, u16 lenOfData, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 10 + lenOfData;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'R';
	out_buffer[ct++] = 0x04;
	
	rhea::utils::bufferWriteU32 (&out_buffer[ct], socketUID);
	ct += 4;

	rhea::utils::bufferWriteU16 (&out_buffer[ct], lenOfData);
	ct += 2;

	if (NULL != data && lenOfData)
    {
        memcpy (&out_buffer[ct], data, lenOfData);
        ct += lenOfData;
    }

    out_buffer[ct] = rhea::utils::simpleChecksum8_calc (out_buffer, ct);
    ct++;

	return ct;
}

//****************************************************************************
u32 esapi::buildMsg_R0x05_getIPandSSID (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('R', 0x05, NULL, 0, out_buffer, sizeOfOutBuffer);
}

//****************************************************************************
u32 esapi::buildMsg_R0x06_start (u8 *out_buffer, u32 sizeOfOutBuffer)
{
	return priv_esapi_buildMsg ('R', 0x06, NULL, 0, out_buffer, sizeOfOutBuffer);
}


//****************************************************************************
void esapi::ask_UNSUBSCRIBE (const cpubridge::sSubscriber &from)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_UNSUBSCRIBE, (u32)0);
}

//****************************************************************************
void esapi::ask_GET_MODULE_TYPE_AND_VER (const cpubridge::sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_GET_MODULE_TYPE_AND_VER, handlerID);
}

//****************************************************************************
void esapi::notify_MODULE_TYPE_AND_VER (const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, eExternalModuleType type, u8 verMajor, u8 verMinor)
{
	logger->log("notify_MODULE_TYPE_AND_VER\n");

	const u8 data[4] = { (u8)type, verMajor, verMinor, 0 };
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, ESAPI_NOTIFY_MODULE_TYPE_AND_VER, handlerID, data, 3);
}

//****************************************************************************
void esapi::translateNotify_MODULE_TYPE_AND_VER(const rhea::thread::sMsg &msg, eExternalModuleType *out_type, u8 *out_verMajor, u8 *out_verMinor)
{
    assert(msg.what == ESAPI_NOTIFY_MODULE_TYPE_AND_VER);

	const u8 *p = (const u8*)msg.buffer;
	*out_type = (eExternalModuleType)p[0];
	*out_verMajor = p[1];
	*out_verMinor = p[2];
}


//****************************************************************************
void esapi::ask_RASPI_GET_WIFI_IPandSSID (const cpubridge::sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_RASPI_GET_IPandSSID, handlerID);
}

//****************************************************************************
void esapi::notify_RASPI_WIFI_IPandSSID (const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger, u8 ipPart0, u8 ipPart1, u8 ipPart2, u8 ipPart3, const char *ssid)
{
	logger->log("notify_RASPI_WIFI_IPandSSID\n");

    u8 data[256];
    data[0] = ipPart0;
    data[1] = ipPart1;
    data[2] = ipPart2;
    data[3] = ipPart3;
    sprintf_s ((char*)&data[4], sizeof(data) - 4, "%s", ssid);
    const u32 len = strlen(ssid);
    data[4 + len] = 0;
    rhea::thread::pushMsg (to.hFromMeToSubscriberW, ESAPI_NOTIFY_RASPI_IPandSSID, handlerID, data, 5+len);
}

//****************************************************************************
void esapi::translateNotify_RASPI_WIFI_IPandSSID(const rhea::thread::sMsg &msg, char *out_ipAddress, u32 sizeof_outIpAddress, char *out_ssid, u32 sizeof_outssid)
{
	assert (msg.what == ESAPI_NOTIFY_RASPI_IPandSSID);
	const u8 *p = (const u8*)msg.buffer;
    sprintf_s (out_ipAddress, sizeof_outIpAddress, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);

    const u32 nToCopy = msg.bufferSize - 4;
    if (sizeof_outssid >= nToCopy)
        memcpy (out_ssid, &p[4], nToCopy);
    else
    {
        DBGBREAK;
        out_ssid[0] = 0;
    }
}

//****************************************************************************
void esapi::ask_RASPI_START (const cpubridge::sSubscriber &from, u16 handlerID)
{
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_RASPI_START, handlerID);
}

//****************************************************************************
void esapi::notify_RASPI_STARTED(const cpubridge::sSubscriber &to, u16 handlerID, rhea::ISimpleLogger *logger)
{
	logger->log("notify_RASPI_STARTED\n");
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, ESAPI_NOTIFY_RASPI_STARTED, handlerID, NULL, 0);
}

//****************************************************************************
void esapi::ask_RASPI_START_FILEUPLOAD (const cpubridge::sSubscriber &from, const u8* const fullFilePathAndName)
{
	const u32 len = rhea::string::utf8::lengthInBytes (fullFilePathAndName);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_RASPI_START_FILEUPLOAD, (u32)0, fullFilePathAndName, len+1);
}

//****************************************************************************
void esapi::translate_RASPI_START_FILEUPLOAD(const rhea::thread::sMsg &msg, const u8 **out_pointerToFullFilePathAndName)
{
	assert (msg.what == ESAPI_ASK_RASPI_START_FILEUPLOAD);
	*out_pointerToFullFilePathAndName = (const u8*)msg.buffer;
}

//****************************************************************************
void esapi::notify_RASPI_FILEUPLOAD(const cpubridge::sSubscriber &to, rhea::ISimpleLogger *logger, eFileUploadStatus status, u32 kbSoFar)
{
	logger->log("notify_RASPI_FILEUPLOAD\n");
	u8 data[8];
	rhea::utils::bufferWriteU32 (data, kbSoFar);
	data[4] = (u8)status;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, ESAPI_NOTIFY_RASPI_FILEUPLOAD, (u32)0, data, 5);
}

//****************************************************************************
void esapi::translateNotify_RASPI_FILEUPLOAD(const rhea::thread::sMsg &msg, eFileUploadStatus *out_status, u32 *out_kbSoFar)
{
	assert (msg.what == ESAPI_NOTIFY_RASPI_FILEUPLOAD);
	const u8 *p = (const u8*)msg.buffer;
	*out_kbSoFar = rhea::utils::bufferReadU32 (p);
	*out_status = (eFileUploadStatus)p[4];
}

//****************************************************************************
void esapi::ask_RASPI_UNZIP (const cpubridge::sSubscriber &from, const u8* const fileName, const u8* const destFolderNoSlashFinale)
{
	const u32 len1 = rhea::string::utf8::lengthInBytes (fileName);
	const u32 len2 = rhea::string::utf8::lengthInBytes (destFolderNoSlashFinale);
	assert (len1 <= 0xff && len2 <= 0xff);

	const u32 totalSizeOfMsg = 2 + (len1 + 1) + (len2 + 1);
	u8 *data = (u8*)RHEAALLOC(rhea::getScrapAllocator(), totalSizeOfMsg);
	data[0] = (u8)len1;
	data[1] = (u8)len2;
	memcpy (&data[2], fileName, len1 + 1);
	memcpy (&data[2+ len1 + 1], destFolderNoSlashFinale, len2 + 1);
	rhea::thread::pushMsg(from.hFromSubscriberToMeW, ESAPI_ASK_RASPI_UNZIP, (u32)0, data, totalSizeOfMsg);
	RHEAFREE(rhea::getScrapAllocator(), data);
}

//****************************************************************************
void esapi::translate_RASPI_UNZIP(const rhea::thread::sMsg &msg, const u8 **out_pointerToFilename, const u8 **out_pointerToFolderNoSlashFinale)
{
	assert (msg.what == ESAPI_ASK_RASPI_UNZIP);
	const u8 *p = (const u8*)msg.buffer;
	const u8 len1 = p[0];
//	const u8 len2 = p[1];
	*out_pointerToFilename = &p[2];
	*out_pointerToFolderNoSlashFinale = &p[2+len1+1];
}

//****************************************************************************
void esapi::notify_RASPI_UNZIP (const cpubridge::sSubscriber &to, rhea::ISimpleLogger *logger, bool bSuccess)
{
	logger->log("notify_RASPI_UNZIP\n");
	u8 data = 0;
	if (bSuccess)
		data = 0x01;
	rhea::thread::pushMsg(to.hFromMeToSubscriberW, ESAPI_NOTIFY_RASPI_UNZIP, (u32)0, &data, 1);
}
//****************************************************************************
void esapi::translateNotify_RASPI_UNZIP(const rhea::thread::sMsg &msg, bool *out_bSuccess)
{
	assert (msg.what == ESAPI_NOTIFY_RASPI_UNZIP);
	const u8 *p = (const u8*)msg.buffer;
	if (p[0] == 0x01)
		*out_bSuccess = true;
	else
		*out_bSuccess = false;
}
