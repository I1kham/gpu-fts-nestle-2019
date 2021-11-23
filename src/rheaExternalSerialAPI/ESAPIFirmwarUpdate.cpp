#include "ESAPIProtocol.h"
#include "ESAPI.h"
#include "../CPUBridge/CPUBridge.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/string/rheaUTF8String.h"

using namespace esapi;

FirmwareUpdate::FirmwareUpdate()
{
	handle = NULL;
	type = eFirmwareUpdateType::None;		// tipo di file
	blockLen = 500;	// lunghezza del blocco
	fileLenMax = fileLenCur = 0;	// lunghezza del file
	fileName = NULL;	// nome del file in corso
}

FirmwareUpdate::~FirmwareUpdate()
// distruttore
{
	Reset();
}

bool FirmwareUpdate::Reset()
{
	if (NULL != handle)
	{
		rhea::fs::fileClose(handle);
		rhea::fs::fileDelete(fileName);
		handle = NULL;
	}

	if (NULL != fileName)
	{
		delete[] fileName;
		fileName = NULL;
	}

	return true;
}

bool FirmwareUpdate::Open(u8 fileType, u32 totalFileLen)
// apertura del file
{
	bool		ret = true;
	const char	*path = (char *)rhea::getPhysicalPathToAppFolder();

	Reset();

	// definire filename
	fileName = new u8[15 + strlen(path)];
	if (0 == fileType)
	{
		type = eFirmwareUpdateType::firmwareCPU;
		sprintf((char *)fileName,"%s/UpdateCPU.CPU", path);
	}
	else if(1 == fileType)
	{
		type = eFirmwareUpdateType::firmwareGPU;
		// definire filename
		fileName = new u8[20];
		sprintf((char*)fileName, "%s/UpdateGPU.GPU", path);
	}
	else
	{
		delete[] fileName;
		fileName = NULL;
		type = eFirmwareUpdateType::None;
		ret = false;
	}

	if (true == ret)
	{
		handle = rhea::fs::fileOpenForWriteBinary(fileName);
		if (NULL == handle)
		{
			type = eFirmwareUpdateType::None;
			delete[] fileName;
			fileName = NULL;
			ret = false;
		}
		else
		{
			fileLenMax = totalFileLen;
			blockCur = 0;
			fileLenCur = 0;
		}
	}

	return ret;
}

bool FirmwareUpdate::Close()
// chiusura ed applicazione delle verifiche
{
	return true;
}

bool FirmwareUpdate::Append(u16 block, u16 bufferLen, u8* buffer)
// aggiunge un blocco
{
	bool	ret;

	if (NULL == handle)
		ret = false;
	else if (blockCur != 0 && blockCur != block)
		ret = false;
	else if (bufferLen + fileLenCur > fileLenMax)
	{
		ret = false;
	}
	else
	{
		u32 written = rhea::fs::fileWrite(handle, buffer, bufferLen);
		if (written != bufferLen)
		{
			Reset();
			ret = false;
		}
		else
		{
			fileLenCur += written;
			ret = true;
		}
	}

	return ret;
}

bool FirmwareUpdate::Complete(u16 blockNr)
// completamento del trasferiemnto file (attiva le routine relative?)
{
	bool ret;

	if (NULL == handle)
		ret = false;
	else if (blockCur != 0 && blockCur != blockNr)
		ret = false;
	else if (fileLenCur != fileLenMax)
		ret = false;
	else
		ret = true;

	rhea::fs::fileClose(handle);
	handle = NULL;

	if(true == ret)
	{
		if (eFirmwareUpdateType::firmwareCPU == type)
		{
		}
		else if (eFirmwareUpdateType::firmwareGPU == type)
		{
		}
		else
		{
			rhea::fs::fileDelete(fileName);
			delete[] fileName;
			ret = false;
		}
	}
	else
		ret = false;

	return ret;
}