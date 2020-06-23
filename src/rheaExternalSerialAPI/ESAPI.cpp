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
		//segnalo che il thread è partito con successo
		rhea::event::fire(init->hEvThreadStarted);
		core.run();
	}

	return 1;
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

u32 esapi::buildMsg_C1_getCPUScreenMsg_resp (const void *lcdMsg, u16 msgLenInBytes, u8 *out_buffer, u32 sizeOfOutBuffer)
{
	const u32 totalSizeOfMsg = 6 + msgLenInBytes;
	if (sizeOfOutBuffer < totalSizeOfMsg)
	{
		DBGBREAK;
		return 0;
	}

    u32 ct = 0;
    out_buffer[ct++] = '#';
    out_buffer[ct++] = 'C';
	out_buffer[ct++] = '1';
	
    rhea::utils::bufferWriteU16_LSB_MSB (&out_buffer[ct], msgLenInBytes);
    ct += 2;
    if (msgLenInBytes > 0)
    {
        memcpy(&out_buffer[ct], lcdMsg, msgLenInBytes);
        ct += msgLenInBytes;
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
		*out_moduleType = eExternalModuleType_unknown;
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
