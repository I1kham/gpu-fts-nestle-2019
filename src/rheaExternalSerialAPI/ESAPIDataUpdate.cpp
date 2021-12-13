#include "ESAPIProtocol.h"
#include "ESAPI.h"
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaUtils.h"
#include "../rheaCommonLib/rheaString.h"
#include "../rheaCommonLib/string/rheaUTF8String.h"
#include "../CPUBridge/CPUBridge.h"

using namespace esapi;

//****************************************************************
DataUpdate::DataUpdate ()
{
	handle = NULL;
	type = eDataUpdateType::None;		// tipo di file
	blockLen = 500;	// lunghezza del blocco
	fileLenMax = fileLenCur = 0;	// lunghezza del file
	fileName = NULL;	// nome del file in corso
}


//****************************************************************
DataUpdate::~DataUpdate()
{
	Reset();
}

//****************************************************************
bool DataUpdate::Reset()
{
	if (NULL != handle)
	{
		rhea::fs::fileClose(handle);
		handle = NULL;
	}

	if (NULL != fileName)
	{
		rhea::fs::fileDelete(fileName);
		RHEAFREE(rhea::getSysHeapAllocator(), fileName);
		fileName = NULL;
	}

	return true;
}

//****************************************************************
bool DataUpdate::Open(u8 fileType, u32 totalFileLen)
// apertura del file
{
	bool		ret = true;
	u8			idx;
	struct
	{
		eDataUpdateType	type;
		char				extension[4];

	}arCross[] = { {eDataUpdateType::CPU, "CPU"},
					{eDataUpdateType::GPU, "GPU"},
					{eDataUpdateType::GUI, "GUI"},
					{eDataUpdateType::DataFile, "DA3"} };

	Reset();

	// definire filename
	for (idx = 0; idx < sizeof arCross / sizeof arCross[0] && (u8)arCross[idx].type != fileType; idx++)
	{
		// empty loop
	}

	if (idx != sizeof arCross / sizeof arCross[0])
	{
		type = arCross[idx].type;

		const u8 *path = rhea::getPhysicalPathToAppFolder();
		const u32 sizeof_filename = 25 + rhea::string::utf8::lengthInBytes(path);
		fileName = RHEAALLOCT(u8*, rhea::getSysHeapAllocator(), sizeof_filename);
		rhea::string::utf8::spf (fileName, sizeof_filename, "%s/temp/Update%s.temp", path, arCross[idx].extension);

		handle = rhea::fs::fileOpenForWriteBinary((u8 *)fileName);
		if (NULL == handle)
		{
			type = eDataUpdateType::None;
			RHEAFREE(rhea::getSysHeapAllocator(), fileName);
			fileName = NULL;
			ret = false;
		}
		else
		{
			fileLenMax = totalFileLen;
			blockCur = 0;
			fileLenCur = 0;
			ret = true;
		}
	}
	else
		ret = false;

	return ret;
}

//****************************************************************
bool DataUpdate::Append(u16 block, u16 bufferLen, u8* buffer)
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

//****************************************************************
bool DataUpdate::Complete(u16 blockNr, const cpubridge::sSubscriber *from)
// completamento del trasferimento file
{
	bool ret;

	if (NULL == handle)
		ret = false;
	else
	{
		rhea::fs::fileClose(handle);
		handle = NULL;

        if (fileLenCur != fileLenMax || (blockCur != 0 && blockCur != blockNr) )
		{
			ret = false;
		}
		else if (eDataUpdateType::CPU == type)
			ret = UpdateCPU(from);
		else if (eDataUpdateType::GPU == type)
			ret = UpdateGPU(from);
		else if (eDataUpdateType::GUI == type)
			ret = UpdateGUI(from);
		else if (eDataUpdateType::DataFile == type)
			ret = UpdateDA3(from);
		else
			ret = false;
	}

	return ret;
}

//****************************************************************
bool DataUpdate::UpdateCPU(const cpubridge::sSubscriber *from)
{
    //cpubridge::ask_WRITE_CPUFW(from, 0, fileName);

    cpubridge::copyFileInAutoupdateFolder (fileName, "cpuFromRheAPI.mhx");
    rhea::fs::fileDelete(fileName);
    ask_SCHEDULE_ACTION_RELAXED_REBOOT (*from, 0x00002);
	return true;
}

//****************************************************************
bool DataUpdate::UpdateGPU(const cpubridge::sSubscriber *from)
{
    cpubridge::copyFileInAutoupdateFolder (fileName, "gpuFromRheAPI.mh6");
    rhea::fs::fileDelete(fileName);
    ask_SCHEDULE_ACTION_RELAXED_REBOOT (*from, 0x00002);
    return true;
}

//****************************************************************
bool DataUpdate::UpdateDA3(const cpubridge::sSubscriber *from)
{
    //cpubridge::ask_WRITE_VMCDATAFILE(from, 0, fileName);
    cpubridge::copyFileInAutoupdateFolder (fileName, "da3FromRheAPI.da3");
    rhea::fs::fileDelete(fileName);
    ask_SCHEDULE_ACTION_RELAXED_REBOOT (*from, 0x00002);
    return true;
}

//****************************************************************
bool DataUpdate::UpdateGUI(const cpubridge::sSubscriber *from)
{
    //la GUI arriva in forma di un file tar. Copio il tar in autoUpdate e poi lo scompatto li
    cpubridge::copyFileInAutoupdateFolder (fileName, "guiFromRheAPI.tar");
    rhea::fs::fileDelete(fileName);


    char src[512];
    sprintf_s (src, sizeof(src), "%s/autoUpdate/guiFromRheAPI.tar", rhea::getPhysicalPathToAppFolder());

    char dstFolder[512];
    sprintf_s (dstFolder, sizeof(dstFolder), "%s/autoUpdate", rhea::getPhysicalPathToAppFolder());

    char cmdLine[256];
    sprintf_s (cmdLine, sizeof(cmdLine), "tar -xf %s -C %s", src, dstFolder);

    char result[32];
    memset (result, 0, sizeof(result));
    rhea::executeShellCommandAndStoreResult (cmdLine, result, sizeof(result));

    ask_SCHEDULE_ACTION_RELAXED_REBOOT (*from, 0x00002);
    return true;
}

